# ADAM on Linux

## Install

Install ADAM running this one-liner:
>
```bash
curl https://imadam.ai/install.sh | sh
```

## Manual install

### Download the `adam` binary

ADAM is distributed as a self-contained binary. Download it to a directory in your PATH:

```bash
sudo curl -L https://imadam.ai/download/adam-linux-amd64 -o /usr/bin/adam
sudo chmod +x /usr/bin/adam
```

### Adding ADAM as a startup service (recommended)

Create a user for ADAM:

```bash
sudo useradd -r -s /bin/false -m -d /usr/share/adam adam
```

Create a service file in `/etc/systemd/system/adam.service`:

```ini
[Unit]
Description=ADAM Service
After=network-online.target

[Service]
ExecStart=/usr/bin/adam serve
User=adam
Group=adam
Restart=always
RestartSec=3

[Install]
WantedBy=default.target
```

Then start the service:

```bash
sudo systemctl daemon-reload
sudo systemctl enable adam
```

### Install CUDA drivers (optional – for Nvidia GPUs)

[Download and install](https://developer.nvidia.com/cuda-downloads) CUDA.

Verify that the drivers are installed by running the following command, which should print details about your GPU:

```bash
nvidia-smi
```

### Start ADAM

Start ADAM using `systemd`:

```bash
sudo systemctl start adam
```

## Update

Update ADAM by running the install script again:

```bash
curl https://imadam.ai/install.sh | sh
```

Or by downloading the adam binary:

```bash
sudo curl -L https://imadam.ai/download/adam-linux-amd64 -o /usr/bin/adam
sudo chmod +x /usr/bin/adam
```

## Viewing logs

To view logs of ADAM running as a startup service, run:

```bash
journalctl -u adam
```

## Uninstall

Remove the adam service:

```bash
sudo systemctl stop adam
sudo systemctl disable adam
sudo rm /etc/systemd/system/adam.service
```

Remove the adam binary from your bin directory (either `/usr/local/bin`, `/usr/bin`, or `/bin`):

```bash
sudo rm $(which adam)
```

Remove the downloaded models and ADAM service user and group:
```bash
sudo rm -r /usr/share/adam
sudo userdel adam
sudo groupdel adam
```
