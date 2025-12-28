# LibreCipher – Design Criptográfico

## Visão Geral

**LibreCipher** é a suite criptográfica própria do LibreCrypt Wallet, projetada para:

- 🔓 Código aberto e auditável
- ⚡ Otimização para microcontroladores (RP2350)
- ⏱️ Operações constant-time
- 🛡️ Resistência a ataques de canal lateral

## Componentes

### 1. LibreCipher-KDF (Key Derivation Function)

**Base**: HKDF (RFC 5869) com modificações

```
Entrada: password, salt, info
Saída: derived_key (256 bits)

1. PRK = HMAC-SHA256(salt, password)
2. OKM = HMAC-SHA256(PRK, info || 0x01)
```

**Parâmetros**:
- Salt: 256 bits (gerado pelo TRNG)
- Info: contexto de uso ("wallet-master", "pin-key", etc.)

### 2. LibreCipher-Hash

**Base**: SHA-256 (FIPS 180-4)

Implementação própria constant-time para evitar dependências externas.

**Vetores de teste**: Compatíveis com NIST CAVP.

### 3. LibreCipher-Sign (Assinatura Digital)

**Fase inicial**: Ed25519 (padrão consolidado)

**Futuro**: Algoritmo de assinatura próprio baseado em curvas elípticas.

```
sign(private_key, message) → signature (64 bytes)
verify(public_key, message, signature) → bool
```

### 4. LibreCipher-Encrypt (Criptografia Simétrica)

**Algoritmo**: AES-256-GCM

```
encrypt(key, nonce, plaintext, aad) → (ciphertext, tag)
decrypt(key, nonce, ciphertext, tag, aad) → plaintext | error
```

**Nonce**: 96 bits, gerado pelo TRNG
**Tag**: 128 bits

## Requisitos de Implementação

### Constant-Time

Todas as operações criptográficas DEVEM ser constant-time:

```c
// ✅ Correto: tempo constante
int secure_compare(const uint8_t *a, const uint8_t *b, size_t len) {
    uint8_t diff = 0;
    for (size_t i = 0; i < len; i++) {
        diff |= a[i] ^ b[i];
    }
    return diff == 0;
}

// ❌ Incorreto: tempo variável
int insecure_compare(const uint8_t *a, const uint8_t *b, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (a[i] != b[i]) return 0; // Early exit!
    }
    return 1;
}
```

### Zero Alocação Dinâmica

```c
// ✅ Stack allocation
uint8_t key[32];

// ❌ Heap allocation (proibido)
uint8_t *key = malloc(32);
```

### Limpeza de Memória

```c
// Após uso de segredos
librecipher_secure_zero(key, sizeof(key));
```

## Integração com Padrões

Durante a fase de desenvolvimento, LibreCipher coexiste com:

| Funcionalidade | LibreCipher | Padrão Fallback |
|---------------|-------------|-----------------|
| Hash | LibreCipher-Hash | SHA-256 |
| KDF | LibreCipher-KDF | HKDF |
| Assinatura | (futuro) | Ed25519 |
| Criptografia | AES-256-GCM | AES-256-GCM |

## Vetores de Teste

Vetores de teste serão publicados em `/tests/crypto/vectors/`.

## Roadmap

1. ✅ Definição inicial do design
2. ⏳ Implementação de referência em C
3. ⏳ Testes com vetores NIST
4. ⏳ Auditoria de segurança
5. ⏳ Algoritmo de assinatura próprio
