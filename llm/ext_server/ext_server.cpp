#include "ext_server.h"

// Necessary evil since the server types are not defined in a header
#include "server.cpp"

// Low level API access to verify GPU access
#if defined(GGML_USE_CUBLAS)
#if defined(GGML_USE_HIPBLAS)
#include <hip/hip_runtime.h>
#include <hipblas/hipblas.h>
#include <hip/hip_fp16.h>
#ifdef __HIP_PLATFORM_AMD__
// for rocblas_initialize()
#include "rocblas/rocblas.h"
#endif // __HIP_PLATFORM_AMD__
#define cudaGetDevice hipGetDevice
#define cudaError_t hipError_t
#define cudaSuccess hipSuccess
#define cudaGetErrorString hipGetErrorString
#else
#include <cuda_runtime.h>
#include <cublas_v2.h>
#include <cuda_fp16.h>
#endif // defined(GGML_USE_HIPBLAS)
#endif // GGML_USE_CUBLAS

// Expose the moses server as a callable extern "C" API
moses_server_context *moses = NULL;
std::atomic<bool> ext_server_running(false);
std::thread ext_server_thread;

void moses_server_init(ext_server_params *sparams, ext_server_resp_t *err) {
  assert(err != NULL && sparams != NULL);
  log_set_target(stderr);
  if (!sparams->verbose_logging) {
    log_disable();
  }

  LOG_TEE("system info: %s\n", moses_print_system_info());
  err->id = 0;
  err->msg[0] = '\0';
  try {
    moses = new moses_server_context;
    gpt_params params;
    params.n_ctx = sparams->n_ctx;
    params.n_batch = sparams->n_batch;
    if (sparams->n_threads > 0) {
      params.n_threads = sparams->n_threads;
    }
    params.n_parallel = sparams->n_parallel;
    params.rope_freq_base = sparams->rope_freq_base;
    params.rope_freq_scale = sparams->rope_freq_scale;

    if (sparams->memory_f16) {
      params.cache_type_k = "f16";
      params.cache_type_v = "f16";
    } else {
      params.cache_type_k = "f32";
      params.cache_type_v = "f32";
    }

    params.n_gpu_layers = sparams->n_gpu_layers;
    params.main_gpu = sparams->main_gpu;
    params.use_mlock = sparams->use_mlock;
    params.use_mmap = sparams->use_mmap;
    params.numa = sparams->numa;
    params.embedding = sparams->embedding;
    if (sparams->model != NULL) {
      params.model = sparams->model;
    }

    if (sparams->lora_adapters != NULL) {
      for (ext_server_lora_adapter *la = sparams->lora_adapters; la != NULL;
          la = la->next) {
        params.lora_adapter.push_back(std::make_tuple(la->adapter, la->scale));
      }

      params.use_mmap = false;
    }

    if (sparams->mmproj != NULL) {
      params.mmproj = std::string(sparams->mmproj);
    }

#if defined(GGML_USE_CUBLAS)
    // Before attempting to init the backend which will assert on error, verify the CUDA/ROCM GPU is accessible
    LOG_TEE("Performing pre-initialization of GPU\n");
    int id;
    cudaError_t cudaErr = cudaGetDevice(&id);
    if (cudaErr != cudaSuccess) {
      err->id = -1;
      snprintf(err->msg, err->msg_len, "Unable to init GPU: %s", cudaGetErrorString(cudaErr));
      return;
    }
#endif

    moses_backend_init(params.numa);

    // load the model
    if (!moses->load_model(params)) {
      // TODO - consider modifying the logging logic or patching load_model so
      // we can capture more detailed error messages and pass them back to the
      // caller for better UX
      err->id = -1;
      snprintf(err->msg, err->msg_len, "error loading model %s",
               params.model.c_str());
      return;
    }

    moses->initialize();
  } catch (std::exception &e) {
    err->id = -1;
    snprintf(err->msg, err->msg_len, "exception %s", e.what());
  } catch (...) {
    err->id = -1;
    snprintf(err->msg, err->msg_len,
             "Unknown exception initializing moses server");
  }
}

void moses_server_start() {
  assert(moses != NULL);
  // TODO mutex to protect thread creation
  ext_server_thread = std::thread([&]() {
    ext_server_running = true;
    try {
      LOG_TEE("moses server main loop starting\n");
      ggml_time_init();
      while (ext_server_running.load()) {
        if (!moses->update_slots()) {
          LOG_TEE(
              "unexpected error in moses server update_slots - exiting main "
              "loop\n");
          break;
        }
      }
    } catch (std::exception &e) {
      LOG_TEE("caught exception in moses server main loop: %s\n", e.what());
    } catch (...) {
      LOG_TEE("caught unknown exception in moses server main loop\n");
    }
    LOG_TEE("\nmoses server shutting down\n");
    moses_backend_free();
  });
}

void moses_server_stop() {
  assert(moses != NULL);
  // TODO - too verbose, remove once things are solid
  LOG_TEE("requesting moses server shutdown\n");
  ext_server_running = false;

  // unblocks the update_slots() loop so it can clean up and exit
  moses->request_cancel(0);

  ext_server_thread.join();
  delete moses;
  moses = NULL;
  LOG_TEE("moses server shutdown complete\n");
}

void moses_server_completion(const char *json_req, ext_server_resp_t *resp) {
  assert(moses != NULL && json_req != NULL && resp != NULL);
  resp->id = -1;
  resp->msg[0] = '\0';
  try {
    json data = json::parse(json_req);
    resp->id = moses->request_completion(data, false, false, -1);
  } catch (std::exception &e) {
    snprintf(resp->msg, resp->msg_len, "exception %s", e.what());
  } catch (...) {
    snprintf(resp->msg, resp->msg_len, "Unknown exception during completion");
  }
}

void moses_server_completion_next_result(const int task_id,
                                         ext_server_task_result_t *resp) {
  assert(moses != NULL && resp != NULL);
  std::string msg;
  resp->id = -1;
  resp->stop = false;
  resp->error = false;
  resp->json_resp = NULL;
  std::string result_json;
  try {
    task_result result = moses->next_result(task_id);
    result_json =
        result.result_json.dump(-1, ' ', false, json::error_handler_t::replace);
    resp->id = result.id;
    resp->stop = result.stop;
    resp->error = result.error;
    if (result.error) {
      moses->request_cancel(task_id);
    } else if (result.stop) {
      moses->request_cancel(task_id);
    }
  } catch (std::exception &e) {
    resp->error = true;
    resp->id = -1;
    result_json = "{\"error\":\"exception " + std::string(e.what()) + "\"}";
    LOG_TEE("moses server completion exception %s\n", e.what());
  } catch (...) {
    resp->error = true;
    resp->id = -1;
    result_json = "{\"error\":\"Unknown exception during completion\"}";
    LOG_TEE("moses server completion unknown exception\n");
  }
  const std::string::size_type size = result_json.size() + 1;
  resp->json_resp = new char[size];
  snprintf(resp->json_resp, size, "%s", result_json.c_str());
}

void moses_server_release_task_result(ext_server_task_result_t *result) {
  if (result == NULL || result->json_resp == NULL) {
    return;
  }
  delete[] result->json_resp;
}

void moses_server_completion_cancel(const int task_id, ext_server_resp_t *err) {
  assert(moses != NULL && err != NULL);
  err->id = 0;
  err->msg[0] = '\0';
  try {
    moses->request_cancel(task_id);
  } catch (std::exception &e) {
    err->id = -1;
    snprintf(err->msg, err->msg_len, "exception %s", e.what());
  } catch (...) {
    err->id = -1;
    snprintf(err->msg, err->msg_len,
             "Unknown exception completion cancel in moses server");
  }
}

void moses_server_tokenize(const char *json_req, char **json_resp,
                           ext_server_resp_t *err) {
  assert(moses != NULL && json_req != NULL && json_resp != NULL && err != NULL);
  *json_resp = NULL;
  err->id = 0;
  err->msg[0] = '\0';
  try {
    const json body = json::parse(json_req);
    std::vector<moses_token> tokens;
    if (body.count("content") != 0) {
      tokens = moses->tokenize(body["content"], false);
    }
    const json data = format_tokenizer_response(tokens);
    std::string result_json = data.dump();
    const std::string::size_type size = result_json.size() + 1;
    *json_resp = new char[size];
    snprintf(*json_resp, size, "%s", result_json.c_str());
  } catch (std::exception &e) {
    err->id = -1;
    snprintf(err->msg, err->msg_len, "exception %s", e.what());
  } catch (...) {
    err->id = -1;
    snprintf(err->msg, err->msg_len, "Unknown exception during tokenize");
  }
}

void moses_server_release_json_resp(char **json_resp) {
  if (json_resp == NULL || *json_resp == NULL) {
    return;
  }
  delete[] *json_resp;
}

void moses_server_detokenize(const char *json_req, char **json_resp,
                             ext_server_resp_t *err) {
  assert(moses != NULL && json_req != NULL && json_resp != NULL && err != NULL);
  *json_resp = NULL;
  err->id = 0;
  err->msg[0] = '\0';
  try {
    const json body = json::parse(json_req);
    std::string content;
    if (body.count("tokens") != 0) {
      const std::vector<moses_token> tokens = body["tokens"];
      content = tokens_to_str(moses->ctx, tokens.cbegin(), tokens.cend());
    }
    const json data = format_detokenized_response(content);
    std::string result_json = data.dump();
    const std::string::size_type size = result_json.size() + 1;
    *json_resp = new char[size];
    snprintf(*json_resp, size, "%s", result_json.c_str());
  } catch (std::exception &e) {
    err->id = -1;
    snprintf(err->msg, err->msg_len, "exception %s", e.what());
  } catch (...) {
    err->id = -1;
    snprintf(err->msg, err->msg_len, "Unknown exception during detokenize");
  }
}

void moses_server_embedding(const char *json_req, char **json_resp,
                            ext_server_resp_t *err) {
  assert(moses != NULL && json_req != NULL && json_resp != NULL && err != NULL);
  *json_resp = NULL;
  err->id = 0;
  err->msg[0] = '\0';
  try {
    const json body = json::parse(json_req);
    json prompt;
    if (body.count("content") != 0) {
      prompt = body["content"];
    } else {
      prompt = "";
    }
    const int task_id = moses->request_completion(
        {{"prompt", prompt}, {"n_predict", 0}}, false, true, -1);
    task_result result = moses->next_result(task_id);
    std::string result_json = result.result_json.dump();
    const std::string::size_type size = result_json.size() + 1;
    *json_resp = new char[size];
    snprintf(*json_resp, size, "%s", result_json.c_str());
  } catch (std::exception &e) {
    err->id = -1;
    snprintf(err->msg, err->msg_len, "exception %s", e.what());
  } catch (...) {
    err->id = -1;
    snprintf(err->msg, err->msg_len, "Unknown exception during embedding");
  }
}