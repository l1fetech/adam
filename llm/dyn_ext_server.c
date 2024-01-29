#include "dyn_ext_server.h"

#include <stdio.h>
#include <string.h>

#ifdef __linux__
#include <dlfcn.h>
#define LOAD_LIBRARY(lib, flags) dlopen(lib, flags)
#define LOAD_SYMBOL(handle, sym) dlsym(handle, sym)
#define LOAD_ERR() strdup(dlerror())
#define UNLOAD_LIBRARY(handle) dlclose(handle)
#elif _WIN32
#include <windows.h>
#define LOAD_LIBRARY(lib, flags) LoadLibrary(lib)
#define LOAD_SYMBOL(handle, sym) GetProcAddress(handle, sym)
#define UNLOAD_LIBRARY(handle) FreeLibrary(handle)
inline char *LOAD_ERR() {
  LPSTR messageBuffer = NULL;
  size_t size = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPSTR)&messageBuffer, 0, NULL);
  char *resp = strdup(messageBuffer);
  LocalFree(messageBuffer);
  return resp;
}
#else
#include <dlfcn.h>
#define LOAD_LIBRARY(lib, flags) dlopen(lib, flags)
#define LOAD_SYMBOL(handle, sym) dlsym(handle, sym)
#define LOAD_ERR() strdup(dlerror())
#define UNLOAD_LIBRARY(handle) dlclose(handle)
#endif

void dyn_init(const char *libPath, struct dynamic_moses_server *s,
                       ext_server_resp_t *err) {
  int i = 0;
  struct lookup {
    char *s;
    void **p;
  } l[] = {
      {"moses_server_init", (void *)&s->moses_server_init},
      {"moses_server_start", (void *)&s->moses_server_start},
      {"moses_server_stop", (void *)&s->moses_server_stop},
      {"moses_server_completion", (void *)&s->moses_server_completion},
      {"moses_server_completion_next_result",
       (void *)&s->moses_server_completion_next_result},
      {"moses_server_completion_cancel",
       (void *)&s->moses_server_completion_cancel},
      {"moses_server_release_task_result",
       (void *)&s->moses_server_release_task_result},
      {"moses_server_tokenize", (void *)&s->moses_server_tokenize},
      {"moses_server_detokenize", (void *)&s->moses_server_detokenize},
      {"moses_server_embedding", (void *)&s->moses_server_embedding},
      {"moses_server_release_json_resp",
       (void *)&s->moses_server_release_json_resp},
      {"", NULL},
  };

  printf("loading library %s\n", libPath);
  s->handle = LOAD_LIBRARY(libPath, RTLD_LOCAL|RTLD_NOW);
  if (!s->handle) {
    err->id = -1;
    char *msg = LOAD_ERR();
    snprintf(err->msg, err->msg_len,
             "Unable to load dynamic server library: %s", msg);
    free(msg);
    return;
  }

  for (i = 0; l[i].p != NULL; i++) {
    *l[i].p = LOAD_SYMBOL(s->handle, l[i].s);
    if (!l[i].p) {
      UNLOAD_LIBRARY(s->handle);
      err->id = -1;
      char *msg = LOAD_ERR();
      snprintf(err->msg, err->msg_len, "symbol lookup for %s failed: %s",
               l[i].s, msg);
      free(msg);
      return;
    }
  }
}

inline void dyn_moses_server_init(struct dynamic_moses_server s,
                                           ext_server_params_t *sparams,
                                           ext_server_resp_t *err) {
  s.moses_server_init(sparams, err);
}

inline void dyn_moses_server_start(struct dynamic_moses_server s) {
  s.moses_server_start();
}

inline void dyn_moses_server_stop(struct dynamic_moses_server s) {
  s.moses_server_stop();
}

inline void dyn_moses_server_completion(struct dynamic_moses_server s,
                                                 const char *json_req,
                                                 ext_server_resp_t *resp) {
  s.moses_server_completion(json_req, resp);
}

inline void dyn_moses_server_completion_next_result(
    struct dynamic_moses_server s, const int task_id,
    ext_server_task_result_t *result) {
  s.moses_server_completion_next_result(task_id, result);
}

inline void dyn_moses_server_completion_cancel(
    struct dynamic_moses_server s, const int task_id, ext_server_resp_t *err) {
  s.moses_server_completion_cancel(task_id, err);
}
inline void dyn_moses_server_release_task_result(
    struct dynamic_moses_server s, ext_server_task_result_t *result) {
  s.moses_server_release_task_result(result);
}

inline void dyn_moses_server_tokenize(struct dynamic_moses_server s,
                                               const char *json_req,
                                               char **json_resp,
                                               ext_server_resp_t *err) {
  s.moses_server_tokenize(json_req, json_resp, err);
}

inline void dyn_moses_server_detokenize(struct dynamic_moses_server s,
                                                 const char *json_req,
                                                 char **json_resp,
                                                 ext_server_resp_t *err) {
  s.moses_server_detokenize(json_req, json_resp, err);
}

inline void dyn_moses_server_embedding(struct dynamic_moses_server s,
                                                const char *json_req,
                                                char **json_resp,
                                                ext_server_resp_t *err) {
  s.moses_server_embedding(json_req, json_resp, err);
}

inline void dyn_moses_server_release_json_resp(
    struct dynamic_moses_server s, char **json_resp) {
  s.moses_server_release_json_resp(json_resp);
}
