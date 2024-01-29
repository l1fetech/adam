# Deploy ADAM to Kubernetes

## Prerequisites

- ADAM: https://imadam.ai/download
- Kubernetes cluster. This example will use Google Kubernetes Engine.

## Steps

1. Create the ADAM namespace, daemon set, and service

    ```bash
    kubectl apply -f cpu.yaml
    ```

1. Port forward the ADAM service to connect and use it locally

    ```bash
    kubectl -n adam port-forward service/adam 11434:80
    ```

1. Pull and run a model, for example `orca-mini:3b`

    ```bash
    adam run orca-mini:3b
    ```

## (Optional) Hardware Acceleration

Hardware acceleration in Kubernetes requires NVIDIA's [`k8s-device-plugin`](https://github.com/NVIDIA/k8s-device-plugin). Follow the link for more details.

Once configured, create a GPU enabled OllADAMama deployment.

```bash
kubectl apply -f gpu.yaml
```
