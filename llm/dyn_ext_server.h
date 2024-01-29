#include <stdlib.h>

#include "ext_server.h"

#ifdef __cplusplus
extern "C" {
#endif
struct dynamic_moses_server {
  void *handle;
  void (*moses_server_init)(ext_server_params_t *sparams,
                            ext_server_resp_t *err);
  void (*moses_server_start)();
  void (*moses_server_stop)();
  void (*moses_server_completion)(const char *json_req,
                                  ext_server_resp_t *resp);
  void (*moses_server_completion_next_result)(const int task_id,
                                              ext_server_task_result_t *result);
  void (*moses_server_completion_cancel)(const int task_id,
                                         ext_server_resp_t *err);
  void (*moses_server_release_task_result)(ext_server_task_result_t *result);
  void (*moses_server_tokenize)(const char *json_req, char **json_resp,
                                ext_server_resp_t *err);
  void (*moses_server_detokenize)(const char *json_req, char **json_resp,
                                  ext_server_resp_t *err);
  void (*moses_server_embedding)(const char *json_req, char **json_resp,
                                 ext_server_resp_t *err);
  void (*moses_server_release_json_resp)(char **json_resp);
};

void dyn_init(const char *libPath, struct dynamic_moses_server *s,
                       ext_server_resp_t *err);

// No good way to call C function pointers from Go so inline the indirection
void dyn_moses_server_init(struct dynamic_moses_server s,
                                    ext_server_params_t *sparams,
                                    ext_server_resp_t *err);

void dyn_moses_server_start(struct dynamic_moses_server s);

void dyn_moses_server_stop(struct dynamic_moses_server s);

void dyn_moses_server_completion(struct dynamic_moses_server s,
                                          const char *json_req,
                                          ext_server_resp_t *resp);

void dyn_moses_server_completion_next_result(
    struct dynamic_moses_server s, const int task_id,
    ext_server_task_result_t *result);

void dyn_moses_server_completion_cancel(struct dynamic_moses_server s,
                                                 const int task_id,
                                                 ext_server_resp_t *err);

void dyn_moses_server_release_task_result(
    struct dynamic_moses_server s, ext_server_task_result_t *result);

void dyn_moses_server_tokenize(struct dynamic_moses_server s,
                                        const char *json_req, char **json_resp,
                                        ext_server_resp_t *err);

void dyn_moses_server_detokenize(struct dynamic_moses_server s,
                                          const char *json_req,
                                          char **json_resp,
                                          ext_server_resp_t *err);

void dyn_moses_server_embedding(struct dynamic_moses_server s,
                                         const char *json_req, char **json_resp,
                                         ext_server_resp_t *err);
void dyn_moses_server_release_json_resp(struct dynamic_moses_server s,
                                                 char **json_resp);

#ifdef __cplusplus
}
#endif