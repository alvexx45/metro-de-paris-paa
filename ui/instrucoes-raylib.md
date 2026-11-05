# Guia de Implementação: Metrô de Paris com C++ e Raylib

Este documento contém as diretrizes técnicas, arquitetura e instruções práticas para implementar o Trabalho Prático de Projeto e Análise de Algoritmos (PAA) utilizando **C++ (arquivo único)** e a biblioteca gráfica **Raylib**.

---

## 1. Regras do Edital & Cuidados Especiais

1. **Arquivo Único (`main.cpp`):**
   * O edital exige que toda a solução (estruturas de dados, parser, 9 algoritmos e interface gráfica) resida em um **único arquivo fonte**.
   * Não divida o código em múltiplos `.cpp` ou `.h` criados pelo grupo na entrega final.
2. **Ambiente de Apresentação:**
   * A biblioteca Raylib **não está instalada** nas máquinas dos laboratórios da universidade.
   * **Obrigatório:** O grupo deve apresentar o trabalho utilizando o **notebook próprio**, o que é expressamente permitido pelo edital.
3. **Threading para Não Travar o Loop Gráfico:**
   * A Raylib opera em modo imediato a 60 FPS (`while (!WindowShouldClose())`). Se a Força Bruta rodar na thread principal e demorar 10 segundos, o sistema operacional acusará que a janela congelou.
   * **Solução:** Disparar os algoritmos de busca pesados em uma thread secundária via `std::thread`, mantendo a interface interativa e com indicador de progresso.

---

## 2. Como Instalar e Compilar

### Linux (Ubuntu / Debian / Fedora)
Instalação dos pacotes de desenvolvimento:
```bash
sudo apt update
sudo apt install libraylib-dev libgl1-mesa-dev libx11-dev
```

Compilação do arquivo único (`main.cpp`):
```bash
g++ -O3 -std=c++17 main.cpp -o metro_paris -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
./metro_paris
```

### Windows (MinGW-w64 / MSYS2)
```bash
pacman -S mingw-w64-x86_64-raylib
g++ -O3 -std=c++17 main.cpp -o metro_paris.exe -lraylib -lopengl32 -lgdi32 -lwinmm -lpthread
```

---

## 3. Arquitetura da Interface (Layout em 2 Zonas)

Como o grafo do metrô possui coordenadas de até $885 \times 1058$, é essencial dividir a tela em duas áreas:

```
+-----------------------------------+-----------------------------------------+
|  BARRA LATERAL FIXA (HUD / UI)    |  ÁREA DO MAPA (COM CAMERA 2D)           |
|  Largura: ~320px                  |  Largura: ~960px (Redimensionável)      |
|                                   |                                         |
|  [Seletor de Arquivo / Teste]     |  - Zoom via Scroll do Mouse             |
|  (3, 5, 7 ou 10 linhas)           |  - Pan (arraste) via Botão Direito      |
|                                   |  - Arestas desenhadas com cor da linha  |
|  [Problema (a) - Maior Ciclo]     |  - Vértices: círculos com nomes         |
|  [ ] Força Bruta                  |  - Destaque visual dos resultados       |
|  [ ] Branch and Bound             |                                         |
|  [ ] Heurística                   |                                         |
|                                   |                                         |
|  [Problema (b) - Guichês/Domin.]  |                                         |
|  ...                              |                                         |
|  [Problema (c) - Manutenção/Cor]  |                                         |
|  ...                              |                                         |
|                                   |                                         |
|  STATUS & MÉTRICAS:               |                                         |
|  - Tempo medido: 45.2 ms          |                                         |
|  - Custo / Resposta encontrada    |                                         |
+-----------------------------------+-----------------------------------------+
```

* A barra lateral deve ser renderizada **fora** do bloco `BeginMode2D() / EndMode2D()`, garantindo que botões e textos fiquem sempre estáticos e nítidos.
* O mapa do metrô deve ser renderizado **dentro** de `BeginMode2D(camera) / EndMode2D()`.

---

## 4. Estruturas de Dados Recomendadas (`main.cpp`)

```cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <atomic>
#include <raylib.h>
#include <raymath.h>

struct Estacao {
    std::string nome;
    Vector2 pos;              // x e y lidos do arquivo
    std::vector<int> vizinhos; // Índices no vetor de estações
};

struct Trecho {
    int u, v;                 // Índices das duas estações
    Color cor;                // Cor associada à linha
    std::string nomeLinha;
};

struct Grafo {
    std::vector<Estacao> estacoes;
    std::unordered_map<std::string, int> mapaNomes; // Nome -> Índice
    std::vector<Trecho> trechos;
    
    void limpar() {
        estacoes.clear();
        mapaNomes.clear();
        trechos.clear();
    }
};
```

---

## 5. Mapeamento de Cores Oficiais das Linhas

Crie uma função simples para traduzir as strings lidas em `*_linhas.txt` para cores RGBA da Raylib:

```cpp
Color ObterCorLinha(const std::string& nomeCor) {
    if (nomeCor == "amarela") return Color{ 255, 206, 0, 255 };   // Linha 1
    if (nomeCor == "azul")    return Color{ 0, 100, 176, 255 };   // Linha 2
    if (nomeCor == "magenta") return Color{ 189, 0, 129, 255 };  // Linha 4
    if (nomeCor == "laranja") return Color{ 245, 156, 0, 255 };  // Linha 5
    if (nomeCor == "verde")   return Color{ 130, 196, 108, 255 };// Linha 6
    if (nomeCor == "rosa")    return Color{ 243, 164, 186, 255 };// Linha 7
    if (nomeCor == "lilas")   return Color{ 197, 163, 202, 255 };// Linha 8
    if (nomeCor == "oliva")   return Color{ 206, 201, 43, 255 }; // Linha 9
    if (nomeCor == "marrom")  return Color{ 141, 94, 42, 255 };   // Linha 11
    if (nomeCor == "roxa")    return Color{ 102, 36, 131, 255 };  // Linha 14
    return LIGHTGRAY;
}
```

---

## 6. Tratamento de Nomes com Acentuação Francesa

A fonte padrão da Raylib inclui apenas ASCII (32 a 126). Nomes como `Charles_de_Gaulle-Étoile` ou `République` podem apresentar erros na renderização.

**Duas alternativas simples:**
1. **Sanitização de texto (mais segura para entrega em arquivo único):**
   Substitua caracteres acentuados por letras comuns no momento da exibição (ex.: `É` $\to$ `E`, `è` $\to$ `e`, `_` $\to$ ` `).
2. **Carregamento de fonte `.ttf` externa com `LoadFontEx`:**
   Exige que um arquivo `.ttf` esteja na pasta, o que requer incluir o arquivo no zip de entrega.

---

## 7. Controle da Câmera 2D (Zoom & Pan)

Para navegar com facilidade pelo mapa com 51 estações:

```cpp
void AtualizarCamera(Camera2D& camera, Rectangle areaVisivel) {
    Vector2 mousePos = GetMousePosition();
    
    // Pan: Arrastar com o botão direito ou botão do meio
    if (CheckCollisionPointRec(mousePos, areaVisivel)) {
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0f / camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }
        
        // Zoom via roda do mouse focado no cursor
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            Vector2 mouseWorldPos = GetScreenToWorld2D(mousePos, camera);
            camera.offset = mousePos;
            camera.target = mouseWorldPos;
            camera.zoom += wheel * 0.12f;
            if (camera.zoom < 0.3f) camera.zoom = 0.3f;
            if (camera.zoom > 3.0f) camera.zoom = 3.0f;
        }
    }
}
```

---

## 8. Execução dos Algoritmos sem Travar a Janela

Use variáveis atômicas e `std::thread` para executar o cálculo e medir o tempo:

```cpp
std::atomic<bool> calculando{ false };
double tempoExecucaoMs = 0.0;
std::string resultadoDescricao = "Pronto";

// Exemplo para disparo de algoritmo assíncrono:
void DispararAlgoritmo(int problema, int metodo, const Grafo& g) {
    if (calculando) return; // Evita disparar múltiplos cálculos simultâneos
    calculando = true;
    resultadoDescricao = "Calculando...";
    
    std::thread([problema, metodo, g]() {
        auto inicio = std::chrono::high_resolution_clock::now();
        
        // Chama a função correspondente:
        // if (problema == 1 && metodo == 1) ForcaBrutaCiclo(g, ...);
        
        auto fim = std::chrono::high_resolution_clock::now();
        tempoExecucaoMs = std::chrono::duration<double, std::milli>(fim - inicio).count();
        calculando = false;
    }).detach();
}
```

---

## 9. Renderização do Grafo e Visualização dos Resultados

### Desenho das Conexões (Arestas)
```cpp
for (const auto& t : grafo.trechos) {
    Vector2 p1 = grafo.estacoes[t.u].pos;
    Vector2 p2 = grafo.estacoes[t.v].pos;
    DrawLineEx(p1, p2, 3.5f, t.cor);
}
```

### Destaque dos Resultados dos 3 Problemas
* **Problema (a) - Maior Ciclo:** Desenhe as arestas do ciclo encontrado com espessura maior (`DrawLineEx(..., 7.0f, GOLD)`) e setas de sentido.
* **Problema (b) - Guichês (Conjunto Dominante):** Desenhe um círculo dourado ou verde pulsante ao redor das estações escolhidas:
  ```cpp
  DrawCircleV(pos, 14.0f, GOLD);
  DrawCircleV(pos, 8.0f, WHITE);
  ```
* **Problema (c) - Manutenção (Coloração de Grafo):** Preencha cada vértice com a cor associada ao seu período de manutenção (Paleta de cores distintas: Vermelho, Ciano, Verde Claro, Laranja, etc.).

---

## 10. Checklist de Implementação

- [ ] Criar estrutura base do `main.cpp` com parsing dos arquivos de `testes_N2/`.
- [ ] Implementar a `Camera2D` para permitir navegação livre no mapa.
- [ ] Construir a barra lateral de botões com `CheckCollisionPointRec`.
- [ ] Conectar os seletores aos 4 conjuntos de teste (`3linhas`, `5linhas`, `7linhas`, `completo`).
- [ ] Implementar a medição de tempo em milissegundos via `<chrono>`.
- [ ] Implementar o destaque visual específico para cada um dos 3 problemas.
- [ ] Validar compilação limpa com `-Wall -Wextra -O3`.
