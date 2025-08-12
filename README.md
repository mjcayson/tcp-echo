# TCP Echo Server & Client

A modular, RAII-safe TCP echo server and client implemented in C++17,  
built with CMake, and packaged with Docker

The project demonstrates:
- Client Handling
  - Proper frame detection
  - Concurrent client support and IO socket multiplexing using select-based reactor design
- Message serialization/deserialization
- Message Handling
  - Login Request
  - Echo Request
- Message Encryption/Decryption
- Modular architecture for future scalability
- Unit tests with GoogleTest.
- Integration tests verifying end-to-end flow.
- Portable architecture using Docker

## Project Structure

tcp-echo
|-lib/ # shared tcp_echo_core library
|-apps/
|---server/ # server binary (links to lib)
|---client/ # client binary (links to lib)
|-scripts/ # helper scripts for build, tests, run
|-tests/ # unit and integration tests
|-docker/ # Dockerfile
|-docker-compose.yml

## Getting Started

### 1. Prerequisites
- Docker Engine 20.10+
- Docker Compose plugin 2.0+

#### Install Docker on Ubuntu 22.04/24.04
Skip if you already have it installed. This section provides a quick and convenient setup guide for Docker in Ubuntu. For other OS, please refer to the official Docker guide: https://docs.docker.com/get-started/get-docker/
```bash
# Install prerequisites
sudo apt update
sudo apt install -y ca-certificates curl gnupg lsb-release

# Add Docker’s official GPG key
sudo mkdir -p /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | \
  sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg

# Add Docker repo
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] \
  https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable" | \
  sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

# Install Docker Engine + Compose plugin
sudo apt update
sudo apt install -y docker-ce docker-ce-cli containerd.io docker-compose-plugin

# Enable and start Docker
sudo systemctl enable docker
sudo systemctl start docker

# Optional: run Docker without sudo
sudo usermod -aG docker $USER
# Log out and back in or run:
newgrp docker
```

### 2. Build the project
```bash
scripts/build.sh
```

### 3. Run unit tests
```bash
scripts/unit-tests.sh
```

### 4. Run integration tests
```bash
scripts/integration-tests.sh
```

### 5. Run manually
```bash
#start the server manually
scripts/server_up.sh

#spawn N number of clients
scripts/spawn_clients.sh 10 #this spawns 10 clients, each runs a login+echo cycle

#(Optional) inspect server logs
docker compose logs -f server

#stop services
scripts/server_down.sh
```