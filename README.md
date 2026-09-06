<p align="center">
  <img src="https://img.shields.io/badge/TFS-1.5.860-blue?style=for-the-badge" alt="TFS Version"/>
  <img src="https://img.shields.io/badge/Language-C%2B%2B17-orange?style=for-the-badge" alt="C++17"/>
  <img src="https://img.shields.io/badge/Protocol-15.25-green?style=for-the-badge" alt="Protocol 15.25"/>
  <img src="https://img.shields.io/badge/License-GPL--2.0-red?style=for-the-badge" alt="License"/>
  <img src="https://img.shields.io/badge/Status-Active-brightgreen?style=for-the-badge" alt="Status"/>
</p>

<h1 align="center">TFS 1.5 Otimized</h1>

<p align="center">
  <b>The Forgotten Server 1.5.860 — Otimizado por Johncorex</b><br>
  <sub>Um servidor OTServer de alta performance, com otimizações de CPU, memória e segurança para ambientes de produção com centenas de jogadores simultâneos.</sub>
</p>

---

## Sobre o Projeto

O **TFS 1.5 Otimized** é um fork do [The Forgotten Server 1.5.860](https://github.com/otland/forgottenserver), o servidor open-source mais confiável do ecossistema Tibia. Este projeto foi construído com foco em **performance, estabilidade e segurança**, aplicando dezenas de otimizações cirúrgicas no código-fonte C++ e scripts Lua para suportar servidores com alto volume de jogadores sem perda de desempenho.

### Agradecimentos

Este projeto não seria possível sem o trabalho dedication dos seguintes desenvolvedores e contribuidores da comunidade:

- **Mark Samman** — Criador original do The Forgotten Server
- **Nekiro** — Maintainer principal e responsável por inúmeras melhorias no TFS
- **Sarah Weskler** — Contribuidora significativa para o ecossistema TFS
- **Comunidade [Otland](https://otland.net/)** — Todos os desenvolvedores que contribuíram com o código-base ao longo dos anos

Obrigado por tornarem o TFS open-source e acessível para toda a comunidade.

---

## Requisitos do Sistema

### Sistema Operacional Recomendado

| Sistema | Versão | Status | Observação |
|---------|--------|--------|------------|
| **Ubuntu Server** | **22.04 LTS (Jammy Jellyfish)** | Recomendado | Melhor compatibilidade com libs modernas, suporte LTS até 2027 |
| **Ubuntu Server** | **20.04 LTS (Focal Fossa)** | Suportado | Compatível, mas GCC pode precisar de atualização |
| **Debian** | **12 (Bookworm)** | Recomendado | Estável, libs atualizadas |
| **Debian** | **11 (Bullseye)** | Suportado | Funcional, mas libs levemente mais antigas |

> **Importante:** Recomendamos **Ubuntu 22.04 LTS** ou **Debian 12 (Bookworm)** para a melhor experiência. Ambos fornecem GCC 12+, Boost 1.74+, MySQL 8.0+ e OpenSSL 3.0+, que são as versões ideais para compilar o TFS 1.5.860.

### Dependências

| Biblioteca | Versão Mínima | Versão Recomendada |
|-----------|---------------|-------------------|
| **GCC** | 11+ | 12+ |
| **CMake** | 3.20+ | 3.25+ |
| **Boost** | 1.70+ | 1.81+ |
| **MySQL/MariaDB** | 5.7+ / 10.3+ | 8.0+ / 10.6+ |
| **OpenSSL** | 1.1.1+ | 3.0+ |
| **PugiXML** | 1.12+ | Sistema |
| **Lua** | 5.1+ | 5.4+ |

---

## Instalação Rápida (Ubuntu 22.04 / Debian 12)

```bash
# 1. Instalar dependências
sudo apt update && sudo apt install -y \
  build-essential cmake libboost-all-dev libmysqlclient-dev \
  libssl-dev libpugixml-dev liblua5.4-dev git

# 2. Clonar o repositório
git clone https://github.com/Johncorex/TFS-1.5-860.git
cd TFS-1.5-860

# 3. Compilar
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/opt/tfs
make -j$(nproc)
sudo make install

# 4. Configurar o banco de dados
sudo mysql -u root < ../schema.sql
sudo mysql -u root tfs < ../world.sql
```

---

## Features e Otimizações

### Performance — Otimizações de CPU

| Otimização | Descrição | Impacto |
|-----------|-----------|---------|
| **getSpectators O(n+m)** | Merge de mapas com unordered_set, eliminação de traversals redundantes | Redução significativa no overhead de视野 |
| **Pathfinder A\* Min-Heap** | Heap binário + heurística Chebyshev + path cache em Creature | Pathfinding 3-5x mais rápido |
| **onDecay Fix O(n)** | Bug de remoção no bucket 0 corrigido, vectors para cache locality | Elimina CPU waste no decay |
| **Monster AI Idle** | Pula think/move se nenhum jogador a 15 SQM no mesmo andar | ~95% dos monstros idle |
| **NPC Idle Optimization** | NPCs só pensam/caminham a 3 SQM no mesmo andar | ~98% dos NPCs idle |
| **Spell Hash Index** | Lookup O(1) via unordered_map para spells sem parâmetro | Cast instantâneo 10x mais rápido |
| **Combat Empty Skip** | Pula iteração de conditionList quando vazia | Elimina 90% dos loops de condition |
| **Field Item Map** | Mapa estático para conversão de field items | Lookup O(1) em vez de switch |

### Segurança — Proteções Anti-Cheat e Anti-DDoS

| Proteção | Descrição |
|---------|-----------|
| **Per-IP Connection Limit** | Máximo 3 conexões por IP |
| **Connection Rate Limiter** | Máximo 20 conexões/segundo por IP |
| **Global Connection Limit** | Máximo 2000 conexões simultâneas |
| **Slowloris Protection** | Timeout 10s para conexões não autenticadas |
| **TCP Acceptor Backlog** | Limite de 128 conexões pendentes |
| **Action Exhaust System** | Cooldown de 200ms em todas as ações não-movimento |
| **Tile Item Cap** | Máximo 100 itens por tile (anti-lag) |
| **Mailbox Parcel Limit** | Máximo 200 itens, nesting depth 5 (anti-loop) |
| **Packet Rate Limit** | Máximo 25 pacotes/segundo por conexão |

### Bug Fixes

| Fix | Descrição |
|-----|-----------|
| **30+ Crash Fixes** | Correções de NULL pointer, assert-in-production, wrong arg indices |
| **Paralysis Fix** | Remoção de walkExhausted incorreto |
| **Blessing Overflow** | Clamp de blessings no banco de dados |
| **Skull Flicker** | Correção do cache de skull |
| **Death Echo Fix** | Correção do multiplicador de dano |
| **Imbuement Decay** | Todas as imbuements pulam decay em PZ |
| **Path Cache Invalidation** | Cache invalidado ao mudar de posição |

### Funcionalidades Customizadas

| Feature | Descrição |
|---------|-----------|
| **Emote Spells** | Sistema de spells por emotes com toggle por jogador |
| **Exhaust System** | Cooldown granular por tipo de ação (200ms padrão, 500ms NPC talk) |

---

## Estrutura do Projeto

```
TFS-1.5-860/
├── src/                    # Código-fonte C++
│   ├── combat.cpp/h        # Sistema de combate (otimizado)
│   ├── creature.cpp/h      # Base creature (path cache, idle)
│   ├── connection.cpp/h    # Conexões (DDoS protection)
│   ├── game.cpp/h          # Game engine principal
│   ├── map.cpp/h           # Mapa (getSpectators, pathfinder)
│   ├── monster.cpp/h       # Monstros (AI idle)
│   ├── npc.cpp/h           # NPCs (idle 3 SQM)
│   ├── player.cpp/h        # Jogadores
│   ├── spells.cpp/h        # Spells (hash index)
│   ├── tile.cpp/h          # Tiles (item cap)
│   └── ...
├── data/                   # Dados do jogo
│   ├── lib/                # Bibliotecas Lua
│   ├── scripts/            # Scripts Lua
│   ├── monsters/           # Definições de monstros
│   ├── npcs/               # Definições de NPCs
│   └── ...
├── data-global/            # Dados globais customizados
├── CMakeLists.txt          # Build system
├── schema.sql              # Schema do banco de dados
└── config.lua              # Configuração principal
```

---

## Configuração

A configuração principal está em `config.lua`. Principais parâmetros:

```lua
-- IP e portas
ip = "0.0.0.0"
loginPort = 7171
gamePort = 7172

-- Limites de jogadores
maxPlayers = 200
maxPlayersOnlinePerAccount = 1

-- Performance
maxPacketsPerSecond = 25
timeToRegenMinuteStamina = 90    -- regen duplicado
timeToRegenMinutePremiumStamina = 180

-- Emote Spells
emoteSpells = false  -- ativar/desativar
```

---

## Contribuindo

Todos são bem-vindos para abrir **issues** e **pull requests** no projeto. Se você encontrou um bug, tem uma sugestão de melhoria ou quer contribuir com código, sinta-se à vontade!

### Como Contribuir

1. Fork o repositório
2. Crie uma branch para sua feature (`git checkout -b feature/minha-feature`)
3. Commit suas mudanças (`git commit -m 'Adiciona minha feature'`)
4. Push para a branch (`git push origin feature/minha-feature`)
5. Abra um Pull Request

### Diretrizes

- Mantenha o estilo de código consistente com o projeto
- Teste suas mudanças antes de submeter
- Documente mudanças significativas no commit message
- Uma feature por pull request

---

## Contato

| Canal | Link |
|-------|------|
| **Email** | [glauber.dev2022@gmail.com](mailto:glauber.dev2022@gmail.com) |
| **WhatsApp** | [Grupo de Discussão](https://chat.whatsapp.com/EWV3dVvS6nt1em7q23FGu7) |
| **GitHub Issues** | [Abrir Issue](https://github.com/Johncorex/TFS-1.5-860/issues) |

---

## Licença

Este projeto é distribuído sob a licença **GPL-2.0**. Consulte o arquivo [LICENSE](LICENSE) para mais detalhes.

---

<p align="center">
  Feito com dedication por <b>Johncorex</b> para a comunidade Tibia.<br>
  <sub>Se este projeto te ajudou, considere dar uma ⭐ no repositório!</sub>
</p>
