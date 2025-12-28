# Contribuindo para o LibreCrypt Wallet

Obrigado por considerar contribuir com o LibreCrypt Wallet! 🔐

## 📋 Código de Conduta

Respeite todos os contribuidores. Foco em segurança, qualidade e colaboração.

## 🛠️ Como Contribuir

### Issues
- Use templates para bugs e features
- Inclua logs e passos para reproduzir

### Pull Requests
1. Fork o repositório
2. Crie uma branch: `git checkout -b feature/nome-da-feature`
3. Faça commits claros e atômicos
4. Submeta o PR com descrição detalhada

## 🔐 Segurança

### Regras para código criptográfico
- **Zero alocação dinâmica** em funções críticas
- Implementações **constant-time** obrigatórias
- Testes com vetores conhecidos
- Documentação de cada primitivo

### Reportando vulnerabilidades
**NÃO** abra issues públicas para vulnerabilidades. 
Envie para: `security@librecrypt.wallet` (quando disponível)

## 📁 Estrutura do Projeto

```
firmware/     → Código C para RP2350
app/          → App Tauri v2 + Rust
docs/         → Documentação técnica
hardware/     → Schematics e PCB
```

## ✅ Checklist de PR

- [ ] Código compila sem warnings
- [ ] Testes passam
- [ ] Documentação atualizada
- [ ] Sem segredos/credenciais

## 📜 Licença

Contribuições são licenciadas sob Apache 2.0, exceto hardware (CERN-OHL).
