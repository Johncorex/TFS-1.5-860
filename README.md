<p align="center">
  <img src="https://img.shields.io/badge/TFS-1.5.860-blue?style=for-the-badge" alt="TFS Version"/>
  <img src="https://img.shields.io/badge/Language-C%2B%2B20-orange?style=for-the-badge" alt="C++20"/>
  <img src="https://img.shields.io/badge/Protocol-8.60-green?style=for-the-badge" alt="Protocol 8.60"/>
  <img src="https://img.shields.io/badge/License-GPL--2.0-red?style=for-the-badge" alt="License"/>
  <img src="https://img.shields.io/badge/Status-Active-brightgreen?style=for-the-badge" alt="Status"/>
</p>

<h1 align="center">TFS 1.5 860 Optimized</h1>

<p align="center">
  <b>The Forgotten Server 1.5 860 — Otimizado por Johncorex</b><br>
  <sub>Um servidor OTServer de alta performance, com otimizações de CPU, memória e segurança para ambientes de produção com centenas de jogadores simultâneos.</sub>
</p>

---

## Sobre o Projeto

O **TFS 1.5.860 Optimized** é um fork do [The Forgotten Server](https://github.com/otland/forgottenserver), adaptado para o protocolo **8.60**.

O projeto possui otimizações no código-fonte C++ e scripts Lua com foco em:

* Performance
* Estabilidade
* Redução de uso de CPU
* Redução de processamento desnecessário
* Segurança
* Suporte a servidores com grande quantidade de jogadores simultâneos
* Compatibilidade com sistemas modernos Linux
* Gerenciamento de dependências através do **vcpkg**

O projeto utiliza **C++20**.

---

# Requisitos do Sistema

## Sistema Operacional

O projeto pode ser compilado em diferentes distribuições Linux modernas.

### Recomendado

| Sistema    | Versão          | Status      |
| ---------- | --------------- | ----------- |
| **Debian** | **13 (Trixie)** | Recomendado |
| **Ubuntu** | **22.04 LTS**   | Recomendado |
| Ubuntu     | 24.04 LTS       | Suportado   |
| Debian     | 12 Bookworm     | Suportado   |

O projeto utiliza CMake e vcpkg para controlar as principais dependências.

---

# Requisitos de Compilação

| Componente         | Requisito                 |
| ------------------ | ------------------------- |
| **C++**            | C++20                     |
| **GCC**            | 11+                       |
| **CMake**          | 3.19+                     |
| **Boost**          | 1.75+ com HTTP habilitado |
| **OpenSSL**        | 3.0+                      |
| **Lua**            | 5.4                       |
| **PugiXML**        | compatível com vcpkg      |
| **MariaDB Client** | através do vcpkg          |
| **fmt**            | 8.1.1+                    |
| **Zlib**           | Sistema/vcpkg             |
| **vcpkg**          | Recomendado               |

As dependências do projeto são definidas no arquivo `vcpkg.json`.

---

# Instalação no Debian 13

Atualize o sistema:

```bash
sudo apt update
sudo apt upgrade -y
```

Instale as ferramentas básicas:

```bash
sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    pkg-config \
    curl \
    zip \
    unzip \
    tar
```

---

# Instalação do vcpkg

Caso o vcpkg ainda não esteja instalado:

```bash
cd /var
sudo git clone https://github.com/microsoft/vcpkg.git
sudo chown -R $USER:$USER /var/vcpkg

cd /var/vcpkg
./bootstrap-vcpkg.sh
```

Configure a variável:

```bash
export VCPKG_ROOT=/var/vcpkg
```

Para deixar permanente:

```bash
echo 'export VCPKG_ROOT=/var/vcpkg' >> ~/.bashrc
source ~/.bashrc
```

Verifique:

```bash
$VCPKG_ROOT/vcpkg version
```

---

# Clonar o Projeto

```bash
cd /var
git clone https://github.com/Johncorex/TFS-1.5-860.git
cd TFS-1.5-860
```

---

# Compilação utilizando vcpkg

O projeto possui integração nativa com vcpkg através do `CMakeLists.txt` e do `CMakePresets.json`.

A maneira recomendada é utilizar o preset `vcpkg`.

Primeiro configure:

```bash
cmake --preset vcpkg
```

Depois compile:

```bash
cmake --build build -j$(nproc)
```

Em máquinas com muitos núcleos, o parâmetro `-j$(nproc)` permite utilizar os processadores disponíveis durante a compilação.

---

# Compilação utilizando caminho explícito do vcpkg

Também é possível informar diretamente o toolchain:

```bash
cmake \
    -S . \
    -B build \
    -DCMAKE_TOOLCHAIN_FILE=/var/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release
```

Depois:

```bash
cmake --build build -j$(nproc)
```

---

# Build Release

Para uma compilação destinada a produção:

```bash
cmake \
    -S . \
    -B build \
    -DCMAKE_TOOLCHAIN_FILE=/var/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_UNITY_BUILD=ON
```

Compile:

```bash
cmake --build build -j$(nproc)
```

O executável será criado dentro do diretório `build`.

Verifique:

```bash
ls -lh build/
```

---

# Limpar e recompilar

Caso seja necessário realizar uma compilação completamente limpa:

```bash
rm -rf build
```

Depois:

```bash
cmake \
    -S . \
    -B build \
    -DCMAKE_TOOLCHAIN_FILE=/var/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release
```

E:

```bash
cmake --build build -j$(nproc)
```

---

# Dependências gerenciadas pelo vcpkg

O arquivo `vcpkg.json` controla as principais dependências do projeto.

Entre elas:

* Boost
* Boost.Asio
* Boost.Iostreams
* Boost.Locale
* Boost.System
* Boost.Variant
* fmt
* MariaDB Client
* OpenSSL
* PugiXML
* Zlib
* Lua 5.4
* Boost.Beast
* Boost.JSON

O suporte HTTP é habilitado por padrão.

---

# Opções do CMake

## HTTP

O suporte HTTP está habilitado por padrão:

```bash
-DHTTP=ON
```

Para desabilitar:

```bash
-DHTTP=OFF
```

## LuaJIT

Por padrão o projeto utiliza Lua 5.4.

Para utilizar LuaJIT:

```bash
-DUSE_LUAJIT=ON
```

## Testes

Para compilar os testes:

```bash
-DBUILD_TESTING=ON
```

## Unity Build

Unity Build está habilitado por padrão:

```bash
-DENABLE_UNITY_BUILD=ON
```

Pode ser desabilitado com:

```bash
-DENABLE_UNITY_BUILD=OFF
```

---

# Exemplo recomendado — Debian 13

Instalação:

```bash
sudo apt update

sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    pkg-config \
    curl \
    zip \
    unzip \
    tar
```

Configuração:

```bash
export VCPKG_ROOT=/var/vcpkg
```

Clone:

```bash
cd /var
git clone https://github.com/Johncorex/TFS-1.5-860.git
cd TFS-1.5-860
```

Configure:

```bash
cmake --preset vcpkg
```

Compile:

```bash
cmake --build build -j$(nproc)
```

---

# Configuração do Servidor

Após a compilação, configure:

```text
config.lua
```

Principais parâmetros:

```lua
ip = "0.0.0.0"

loginPort = 7171
gamePort = 7172

maxPlayers = 200
maxPlayersOnlinePerAccount = 1

maxPacketsPerSecond = 25

emoteSpells = false
```

Ajuste esses valores de acordo com a infraestrutura e configuração do servidor.

---

# Performance

O projeto contém diversas otimizações destinadas à redução do processamento desnecessário.

Entre elas:

* Otimizações de `getSpectators`
* Otimização do pathfinding
* Path cache
* Otimização do sistema de decay
* Otimização da AI de monstros
* Otimização da AI de NPCs
* Índice/hash para busca de spells
* Redução de loops desnecessários no sistema de combate
* Otimizações relacionadas a itens e tiles

---

# Segurança e Limites de Conexão

O projeto possui mecanismos internos de proteção e limitação de conexões.

Entre eles:

* Limitação de conexões por IP
* Limitação de taxa de novas conexões
* Limite global de conexões
* Proteção contra conexões lentas
* Controle do backlog TCP
* Limitação de pacotes por conexão
* Limitação de ações
* Limitação de itens por tile
* Proteções contra loops de containers

> Esses mecanismos devem ser utilizados em conjunto com firewall, proteção DDoS e configuração adequada da infraestrutura.

---

# Estrutura do Projeto

```text
TFS-1.5-860/
├── src/                    # Código-fonte C++
├── data/                   # Dados do jogo
│   ├── lib/
│   ├── scripts/
│   ├── monsters/
│   ├── npcs/
│   └── ...
├── cmake/                  # Módulos CMake
├── CMakeLists.txt          # Build system
├── CMakePresets.json       # Presets do CMake
├── vcpkg.json              # Dependências do projeto
├── schema.sql              # Banco de dados
└── config.lua              # Configuração do servidor
```

---

# Banco de Dados

O projeto utiliza MySQL/MariaDB.

O schema do banco está disponível em:

```text
schema.sql
```

Exemplo:

```bash
mysql -u root -p database_name < schema.sql
```

> Substitua `database_name` pelo nome do banco utilizado pelo servidor.

---

# Protocolo

Este projeto é destinado ao protocolo:

```text
Tibia 8.60
```

Clientes compatíveis devem estar configurados corretamente para o protocolo utilizado pelo servidor.

---

# Desenvolvimento

Para desenvolver novas funcionalidades, recomenda-se trabalhar em uma branch:

```bash
git checkout -b feature/minha-feature
```

Após realizar as alterações:

```bash
git add .
git commit -m "Minha alteração"
git push origin feature/minha-feature
```

Pull Requests são bem-vindos.

---

# Licença

Este projeto é distribuído sob a licença **GPL-2.0**.

Consulte o arquivo:

```text
LICENSE
```

para obter os termos completos da licença.

---

<p align="center">
  Feito com dedicação por <b>Johncorex</b> para a comunidade Tibia.
</p>
