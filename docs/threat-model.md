# Modelo de Ameaças – LibreCrypt Wallet

## Escopo

Este documento define as ameaças consideradas e as mitigações implementadas.

## Ativos Protegidos

| Ativo | Criticidade | Descrição |
|-------|-------------|-----------|
| Seed/Chaves Privadas | 🔴 Crítico | Chaves criptográficas da carteira |
| PIN do Usuário | 🟠 Alto | Autenticação local |
| Transações | 🟠 Alto | Operações financeiras |
| Firmware | 🟡 Médio | Código executável |

## Ameaças e Mitigações

### T1: Malware no Computador Host

**Cenário**: Malware tenta roubar chaves ou manipular transações.

**Mitigações**:
- ✅ Chaves nunca saem do hardware
- ✅ Confirmação física obrigatória no dispositivo
- ✅ Exibição de detalhes no display do hardware

### T2: Ataques Físicos ao Hardware

**Cenário**: Atacante tem acesso físico ao dispositivo.

**Mitigações**:
- ✅ Flash criptografada
- ✅ PIN com rate limiting (wipe após N tentativas)
- ✅ Proteção contra leitura via debug (SWD desabilitado)

### T3: Firmware Malicioso

**Cenário**: Usuário instala firmware comprometido.

**Mitigações**:
- ✅ Secure Boot com verificação de assinatura
- ✅ Anti-rollback (versões antigas bloqueadas)
- ✅ Confirmação física para atualização

### T4: Ataques de Canal Lateral

**Cenário**: Análise de timing, consumo de energia, ou radiação EM.

**Mitigações**:
- ✅ Implementações constant-time no LibreCipher
- ✅ Masking em operações sensíveis
- ✅ RNG para jitter artificial

### T5: Supply Chain Attack

**Cenário**: Hardware modificado antes de chegar ao usuário.

**Mitigações**:
- ⚠️ Verificação de integridade no boot
- ⚠️ Código aberto para auditoria
- ⚠️ Community builds verificáveis

## Matriz de Ameaças

| Ameaça | Probabilidade | Impacto | Mitigação |
|--------|---------------|---------|-----------|
| T1 | Alta | Crítico | Isolamento HW |
| T2 | Média | Crítico | Criptografia + PIN |
| T3 | Baixa | Crítico | Secure Boot |
| T4 | Baixa | Alto | Constant-time |
| T5 | Baixa | Crítico | Auditoria |

## Limites do Modelo

Este modelo **não** protege contra:
- Usuário coagido fisicamente
- Ataques de state-level com recursos ilimitados
- Falhas 0-day em primitivos criptográficos padrão
