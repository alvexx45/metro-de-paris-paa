/**
 * Pontificia Universidade Catolica de Minas Gerais - ICEI
 * Disciplina: Projeto e Analise de Algoritmos (PAA)
 * Professor: Alexei Machado
 *
 * Trabalho Prático: O Metrô de Paris
 * Critério do Grupo: N = 2 (Linhas: 1, 2, 4, 5, 6, 7, 8, 9, 11, 14)
 *
 * Integrantes do Grupo:
 *  - Bernardo Alves Bicalho Varges
 *  - Gabriel Henrique Pereira Carvalhaes
 *  - Mateus Fasciani Viggiani Rios de Castro
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <thread>
#include <atomic>
#include <queue>
#include <numeric>
#include <iomanip>

#include <raylib.h>
#include <raymath.h>

struct Estacao {
    std::string nomeOriginal;
    std::string nomeLimpo;     // Sem underscores e com caracteres seguros para exibição
    Vector2 pos;               // Coordenadas lidas do arquivo
    std::vector<int> vizinhos; // Lista de adjacência (sem duplicatas)
    std::vector<std::string> linhas;
};

struct Trecho {
    int u, v;
    Color cor;
    std::string nomeLinha;
};

struct Grafo {
    std::vector<Estacao> estacoes;
    std::unordered_map<std::string, int> mapaNomes;
    std::vector<Trecho> trechos;
    std::vector<std::vector<bool>> matrizAdj;

    void limpar() {
        estacoes.clear();
        mapaNomes.clear();
        trechos.clear();
        matrizAdj.clear();
    }

    int obterOuCriarIndice(const std::string& nome) {
        auto it = mapaNomes.find(nome);
        if (it != mapaNomes.end()) return it->second;
        int idx = (int)estacoes.size();
        mapaNomes[nome] = idx;
        Estacao e;
        e.nomeOriginal = nome;
        e.nomeLimpo = sanitizarNome(nome);
        e.pos = { 0, 0 };
        estacoes.push_back(e);
        return idx;
    }

    static std::string sanitizarNome(const std::string& raw) {
        std::string res;
        for (char c : raw) {
            if (c == '_') res += ' ';
            else res += c;
        }
        return res;
    }

    static Color converterCor(const std::string& nomeCor) {
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

    bool carregar(const std::string& arqEstacoes, const std::string& arqLinhas) {
        limpar();

        // 1. Ler Estações
        std::ifstream fEst(arqEstacoes);
        if (!fEst.is_open()) {
            std::cerr << "Erro ao abrir arquivo: " << arqEstacoes << std::endl;
            return false;
        }

        std::string linha;
        while (std::getline(fEst, linha)) {
            if (linha.empty() || linha[0] == '#') continue;
            std::stringstream ss(linha);
            std::string nome;
            float x, y;
            if (ss >> nome >> x >> y) {
                int idx = obterOuCriarIndice(nome);
                estacoes[idx].pos = { x, y };
            }
        }
        fEst.close();

        // 2. Ler Linhas e Trechos
        std::ifstream fLin(arqLinhas);
        if (!fLin.is_open()) {
            std::cerr << "Erro ao abrir arquivo: " << arqLinhas << std::endl;
            return false;
        }

        std::string idLinha, corStr;
        int nTrechos = 0;

        while (fLin >> idLinha >> corStr >> nTrechos) {
            Color corLinha = converterCor(corStr);
            for (int i = 0; i < nTrechos; ++i) {
                std::string st1, st2;
                fLin >> st1 >> st2;
                int u = obterOuCriarIndice(st1);
                int v = obterOuCriarIndice(st2);

                trechos.push_back(Trecho{ u, v, corLinha, idLinha });

                // Adiciona às linhas atendidas
                if (std::find(estacoes[u].linhas.begin(), estacoes[u].linhas.end(), idLinha) == estacoes[u].linhas.end())
                    estacoes[u].linhas.push_back(idLinha);
                if (std::find(estacoes[v].linhas.begin(), estacoes[v].linhas.end(), idLinha) == estacoes[v].linhas.end())
                    estacoes[v].linhas.push_back(idLinha);
            }
        }
        fLin.close();

        // 3. Montar grafo simples (matriz e listas sem repetição para os algoritmos)
        int n = (int)estacoes.size();
        matrizAdj.assign(n, std::vector<bool>(n, false));

        for (const auto& t : trechos) {
            if (t.u != t.v && !matrizAdj[t.u][t.v]) {
                matrizAdj[t.u][t.v] = matrizAdj[t.v][t.u] = true;
                estacoes[t.u].vizinhos.push_back(t.v);
                estacoes[t.v].vizinhos.push_back(t.u);
            }
        }

        return true;
    }
};

// ============================================================================
// RESULTADOS DOS ALGORITMOS
// ============================================================================

enum class TipoProblema {
    MAIOR_CICLO = 0,
    GUICHES = 1,
    MANUTENCAO = 2
};

enum class MetodoAlgoritmo {
    FORCA_BRUTA = 0,
    BRANCH_AND_BOUND = 1,
    HEURISTICA = 2
};

struct Resultado {
    TipoProblema problema;
    MetodoAlgoritmo metodo;
    double tempoMs = 0.0;
    std::string descricao;
    
    // Problema (a): Sequência de vértices do ciclo
    std::vector<int> ciclo;

    // Problema (b): Vértices com guichê
    std::vector<int> guiches;

    // Problema (c): Cor de cada vértice (período 1..K)
    std::vector<int> cores;
    int numCores = 0;

    void limpar() {
        tempoMs = 0.0;
        descricao.clear();
        ciclo.clear();
        guiches.clear();
        cores.clear();
        numCores = 0;
    }
};

// ============================================================================
// ESPAÇO PARA IMPLEMENTAÇÃO DOS ALGORITMOS PELO GRUPO
// ============================================================================

// Problema (a): Determinar o número máximo de estações que um turista poderá
// visitar com um só passe, saindo e retornando de uma determinada estação
// (Maior ciclo simples contendo uma estação inicial)
namespace SolucoesCiclo {

    std::vector<int> forcaBruta(const Grafo& g, int inicio) {
        // TODO: Implementar pelo grupo
        (void)g; (void)inicio;
        return {};
    }

    std::vector<int> branchAndBound(const Grafo& g, int inicio) {
        // TODO: Implementar pelo grupo
        (void)g; (void)inicio;
        return {};
    }

    std::vector<int> heuristica(const Grafo& g, int inicio) {
        // TODO: Implementar pelo grupo (complexidade polinomial no pior caso)
        (void)g; (void)inicio;
        return {};
    }
}

// Problema (b): Determinar as estações para instalação de guichês para venda de
// passes, de modo que não seja preciso caminhar mais que uma estação para achar um guichê
// (Menor conjunto dominante)
namespace SolucoesGuiches {

    std::vector<int> forcaBruta(const Grafo& g) {
        // TODO: Implementar pelo grupo
        (void)g;
        return {};
    }

    std::vector<int> branchAndBound(const Grafo& g) {
        // TODO: Implementar pelo grupo
        (void)g;
        return {};
    }

    std::vector<int> heuristica(const Grafo& g) {
        // TODO: Implementar pelo grupo (complexidade polinomial no pior caso)
        (void)g;
        return {};
    }
}

// Problema (c): Determinar o menor número de períodos de manutenção necessários
// de forma que estações conectadas diretamente pela mesma linha não fiquem em manutenção juntas
// (Coloração mínima de vértices / número cromático)
namespace SolucoesColoracao {

    std::pair<int, std::vector<int>> forcaBruta(const Grafo& g) {
        // TODO: Implementar pelo grupo
        (void)g;
        return { 0, {} };
    }

    std::pair<int, std::vector<int>> branchAndBound(const Grafo& g) {
        // TODO: Implementar pelo grupo
        (void)g;
        return { 0, {} };
    }

    std::pair<int, std::vector<int>> heuristica(const Grafo& g) {
        // TODO: Implementar pelo grupo (complexidade polinomial no pior caso)
        (void)g;
        return { 0, {} };
    }
}

// ============================================================================
// APLICAÇÃO E INTERFACE GRÁFICA (RAYLIB)
// ============================================================================

class AppMetro {
public:
    Grafo grafo;
    Camera2D camera;
    Rectangle areaMapa;
    Rectangle barraLateral;

    // Estado da seleção
    int datasetAtual = 3; // 0: 3linhas, 1: 5linhas, 2: 7linhas, 3: completo
    TipoProblema problemaSelecionado = TipoProblema::MAIOR_CICLO;
    MetodoAlgoritmo metodoSelecionado = MetodoAlgoritmo::FORCA_BRUTA;
    int estacaoInicialCiclo = -1; // Vértice inicial para o problema (a)

    // Estado da execução assíncrona
    std::atomic<bool> calculando{ false };
    Resultado resultado;
    int hoverEstacao = -1;

    // Fonte personalizada (Ubuntu)
    Font fonteUbuntu;
    bool fonteCarregada = false;

    // Paleta de cores para manutenção (coloração)
    std::vector<Color> paletaPeriodos = {
        Color{ 230, 57, 70, 255 },   // Vermelho
        Color{ 69, 123, 157, 255 },  // Azul
        Color{ 42, 157, 143, 255 },  // Verde Água
        Color{ 244, 162, 97, 255 },  // Laranja
        Color{ 155, 93, 229, 255 },  // Roxo
        Color{ 247, 37, 133, 255 },  // Rosa
        Color{ 76, 201, 240, 255 },  // Ciano
        Color{ 254, 217, 37, 255 }   // Amarelo
    };

    AppMetro() {
        camera.zoom = 0.85f;
        camera.offset = { 650.0f, 380.0f };
        camera.target = { 460.0f, 480.0f };
        camera.rotation = 0.0f;
    }

    void carregarFonte() {
        const char* caminhos[] = {
            "assets/Ubuntu-Regular.ttf",
            "../assets/Ubuntu-Regular.ttf",
            "Ubuntu-Regular.ttf",
            "../Ubuntu-Regular.ttf"
        };
        for (const char* caminho : caminhos) {
            if (FileExists(caminho)) {
                std::vector<int> codepoints;
                // ASCII básico (32..126)
                for (int i = 32; i <= 126; ++i) codepoints.push_back(i);
                // Suplemento Latin-1 (160..255) cobre acentuação francesa e portuguesa
                for (int i = 160; i <= 255; ++i) codepoints.push_back(i);
                // Caracteres e pontuações especiais
                codepoints.push_back(0x0152); // Œ
                codepoints.push_back(0x0153); // œ
                codepoints.push_back(0x2019); // ’
                codepoints.push_back(0x2018); // ‘
                codepoints.push_back(0x2013); // –
                codepoints.push_back(0x2014); // —

                fonteUbuntu = LoadFontEx(caminho, 42, codepoints.data(), (int)codepoints.size());
                if (fonteUbuntu.texture.id != 0) {
                    SetTextureFilter(fonteUbuntu.texture, TEXTURE_FILTER_BILINEAR);
                    fonteCarregada = true;
                    break;
                }
            }
        }
    }

    void descarregarFonte() {
        if (fonteCarregada) {
            UnloadFont(fonteUbuntu);
            fonteCarregada = false;
        }
    }

    void desenharTexto(const std::string& texto, float x, float y, float tam, Color cor, float espacamento = 1.0f) {
        if (fonteCarregada) {
            DrawTextEx(fonteUbuntu, texto.c_str(), { x, y }, tam, espacamento, cor);
        } else {
            DrawText(texto.c_str(), (int)x, (int)y, (int)tam, cor);
        }
    }

    Vector2 medirTexto(const std::string& texto, float tam, float espacamento = 1.0f) {
        if (fonteCarregada) {
            return MeasureTextEx(fonteUbuntu, texto.c_str(), tam, espacamento);
        } else {
            return { (float)MeasureText(texto.c_str(), (int)tam), tam };
        }
    }

    void carregarDataset(int idx) {
        datasetAtual = idx;
        std::string prefixo;
        if (idx == 0) prefixo = "testes_N2/3linhas";
        else if (idx == 1) prefixo = "testes_N2/5linhas";
        else if (idx == 2) prefixo = "testes_N2/7linhas";
        else prefixo = "testes_N2/completo";

        grafo.carregar(prefixo + "_estacoes.txt", prefixo + "_linhas.txt");
        resultado.limpar();

        // Encontra uma boa estação inicial para o ciclo (ex.: Châtelet ou maior grau)
        estacaoInicialCiclo = 0;
        int maxGrau = -1;
        for (int i = 0; i < (int)grafo.estacoes.size(); ++i) {
            if (grafo.estacoes[i].nomeOriginal.find("Châtelet") != std::string::npos ||
                grafo.estacoes[i].nomeOriginal.find("Chatelet") != std::string::npos) {
                estacaoInicialCiclo = i;
                break;
            }
            if ((int)grafo.estacoes[i].vizinhos.size() > maxGrau) {
                maxGrau = (int)grafo.estacoes[i].vizinhos.size();
                estacaoInicialCiclo = i;
            }
        }
    }

    void executarAlgoritmo() {
        if (calculando) return;
        calculando = true;
        resultado.limpar();
        resultado.problema = problemaSelecionado;
        resultado.metodo = metodoSelecionado;

        // Dispara em thread separada para não congelar o loop a 60 FPS
        std::thread([this]() {
            auto tInicio = std::chrono::high_resolution_clock::now();

            if (problemaSelecionado == TipoProblema::MAIOR_CICLO) {
                int s = (estacaoInicialCiclo >= 0 && estacaoInicialCiclo < (int)grafo.estacoes.size()) ? estacaoInicialCiclo : 0;
                if (metodoSelecionado == MetodoAlgoritmo::FORCA_BRUTA) {
                    resultado.ciclo = SolucoesCiclo::forcaBruta(grafo, s);
                } else if (metodoSelecionado == MetodoAlgoritmo::BRANCH_AND_BOUND) {
                    resultado.ciclo = SolucoesCiclo::branchAndBound(grafo, s);
                } else {
                    resultado.ciclo = SolucoesCiclo::heuristica(grafo, s);
                }
                if (resultado.ciclo.empty()) {
                    resultado.descricao = "Aguardando implementação...";
                } else {
                    resultado.descricao = "Ciclo com " + std::to_string(resultado.ciclo.size() - 1) + " estações";
                }
            }
            else if (problemaSelecionado == TipoProblema::GUICHES) {
                if (metodoSelecionado == MetodoAlgoritmo::FORCA_BRUTA) {
                    resultado.guiches = SolucoesGuiches::forcaBruta(grafo);
                } else if (metodoSelecionado == MetodoAlgoritmo::BRANCH_AND_BOUND) {
                    resultado.guiches = SolucoesGuiches::branchAndBound(grafo);
                } else {
                    resultado.guiches = SolucoesGuiches::heuristica(grafo);
                }
                if (resultado.guiches.empty()) {
                    resultado.descricao = "Aguardando implementação...";
                } else {
                    resultado.descricao = std::to_string(resultado.guiches.size()) + " guichês necessários";
                }
            }
            else if (problemaSelecionado == TipoProblema::MANUTENCAO) {
                std::pair<int, std::vector<int>> resCor;
                if (metodoSelecionado == MetodoAlgoritmo::FORCA_BRUTA) {
                    resCor = SolucoesColoracao::forcaBruta(grafo);
                } else if (metodoSelecionado == MetodoAlgoritmo::BRANCH_AND_BOUND) {
                    resCor = SolucoesColoracao::branchAndBound(grafo);
                } else {
                    resCor = SolucoesColoracao::heuristica(grafo);
                }
                resultado.numCores = resCor.first;
                resultado.cores = resCor.second;
                if (resultado.numCores == 0) {
                    resultado.descricao = "Aguardando implementação...";
                } else {
                    resultado.descricao = "Mínimo de " + std::to_string(resultado.numCores) + " períodos (cores)";
                }
            }

            auto tFim = std::chrono::high_resolution_clock::now();
            resultado.tempoMs = std::chrono::duration<double, std::milli>(tFim - tInicio).count();
            calculando = false;
        }).detach();
    }

    void atualizar() {
        Vector2 mousePos = GetMousePosition();

        // Controle da câmera (Pan & Zoom)
        if (CheckCollisionPointRec(mousePos, areaMapa)) {
            // Pan via botão direito ou botão do meio
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
                Vector2 delta = GetMouseDelta();
                delta = Vector2Scale(delta, -1.0f / camera.zoom);
                camera.target = Vector2Add(camera.target, delta);
            }

            // Zoom via scroll da rodinha
            float wheel = GetMouseWheelMove();
            if (wheel != 0) {
                Vector2 mouseWorldPos = GetScreenToWorld2D(mousePos, camera);
                camera.offset = mousePos;
                camera.target = mouseWorldPos;
                camera.zoom += wheel * 0.12f;
                if (camera.zoom < 0.25f) camera.zoom = 0.25f;
                if (camera.zoom > 3.5f) camera.zoom = 3.5f;
            }

            // Hover sobre as estações
            Vector2 worldMouse = GetScreenToWorld2D(mousePos, camera);
            hoverEstacao = -1;
            for (int i = 0; i < (int)grafo.estacoes.size(); ++i) {
                if (Vector2Distance(worldMouse, grafo.estacoes[i].pos) <= 12.0f) {
                    hoverEstacao = i;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && problemaSelecionado == TipoProblema::MAIOR_CICLO) {
                        estacaoInicialCiclo = i;
                    }
                    break;
                }
            }
        }
    }

    // Desenha botão retangular interativo
    bool desenharBotao(Rectangle rect, const char* texto, bool ativo, bool desabilitado = false) {
        Vector2 mouse = GetMousePosition();
        bool hover = CheckCollisionPointRec(mouse, rect) && !desabilitado;
        Color bg = ativo ? Color{ 78, 113, 255, 255 } : (hover ? Color{ 55, 55, 80, 255 } : Color{ 36, 36, 54, 255 });
        if (desabilitado) bg = Color{ 28, 28, 40, 255 };

        DrawRectangleRec(rect, bg);
        DrawRectangleLinesEx(rect, 1.5f, ativo ? Color{ 140, 170, 255, 255 } : Color{ 70, 70, 95, 255 });

        float txtSize = 14.0f;
        Vector2 txtDim = medirTexto(texto, txtSize);
        desenharTexto(texto, rect.x + (rect.width - txtDim.x) / 2.0f, rect.y + (rect.height - txtDim.y) / 2.0f,
                      txtSize, desabilitado ? GRAY : WHITE);

        return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    }

    void desenharUI() {
        // Barra Lateral Fixa
        DrawRectangleRec(barraLateral, Color{ 24, 24, 36, 255 });
        DrawLineEx({ barraLateral.width, 0 }, { barraLateral.width, (float)GetScreenHeight() }, 2.0f, Color{ 45, 45, 65, 255 });

        float y = 20.0f;
        float x = 20.0f;
        float w = barraLateral.width - 40.0f;

        // Cabeçalho
        desenharTexto("METRÔ DE PARIS", x, y, 22.0f, Color{ 255, 206, 0, 255 });
        y += 28.0f;
        desenharTexto("PAA - Grupo N=2", x, y, 14.0f, LIGHTGRAY);
        y += 30.0f;

        // Seção: Conjunto de Teste
        desenharTexto("1. CONJUNTO DE LINHAS:", x, y, 13.0f, Color{ 160, 180, 240, 255 });
        y += 22.0f;

        float bw4 = (w - 15.0f) / 4.0f;
        if (desenharBotao({ x, y, bw4, 28 }, "3 L", datasetAtual == 0, calculando)) carregarDataset(0);
        if (desenharBotao({ x + bw4 + 5, y, bw4, 28 }, "5 L", datasetAtual == 1, calculando)) carregarDataset(1);
        if (desenharBotao({ x + (bw4 + 5) * 2, y, bw4, 28 }, "7 L", datasetAtual == 2, calculando)) carregarDataset(2);
        if (desenharBotao({ x + (bw4 + 5) * 3, y, bw4, 28 }, "10 L", datasetAtual == 3, calculando)) carregarDataset(3);
        y += 40.0f;

        // Seção: Problema
        desenharTexto("2. SELECIONE O PROBLEMA:", x, y, 13.0f, Color{ 160, 180, 240, 255 });
        y += 22.0f;

        if (desenharBotao({ x, y, w, 32 }, "(a) Maior Ciclo Simples", problemaSelecionado == TipoProblema::MAIOR_CICLO, calculando)) {
            problemaSelecionado = TipoProblema::MAIOR_CICLO;
            resultado.limpar();
        }
        y += 36.0f;

        if (desenharBotao({ x, y, w, 32 }, "(b) Guichês (Dominante)", problemaSelecionado == TipoProblema::GUICHES, calculando)) {
            problemaSelecionado = TipoProblema::GUICHES;
            resultado.limpar();
        }
        y += 36.0f;

        if (desenharBotao({ x, y, w, 32 }, "(c) Manutenção (Coloração)", problemaSelecionado == TipoProblema::MANUTENCAO, calculando)) {
            problemaSelecionado = TipoProblema::MANUTENCAO;
            resultado.limpar();
        }
        y += 44.0f;

        // Seção: Método / Abordagem
        desenharTexto("3. MÉTODO DE SOLUÇÃO:", x, y, 13.0f, Color{ 160, 180, 240, 255 });
        y += 22.0f;

        float bw3 = (w - 10.0f) / 3.0f;
        if (desenharBotao({ x, y, bw3, 30 }, "F. Bruta", metodoSelecionado == MetodoAlgoritmo::FORCA_BRUTA, calculando))
            metodoSelecionado = MetodoAlgoritmo::FORCA_BRUTA;
        if (desenharBotao({ x + bw3 + 5, y, bw3, 30 }, "B & B", metodoSelecionado == MetodoAlgoritmo::BRANCH_AND_BOUND, calculando))
            metodoSelecionado = MetodoAlgoritmo::BRANCH_AND_BOUND;
        if (desenharBotao({ x + (bw3 + 5) * 2, y, bw3, 30 }, "Heurística", metodoSelecionado == MetodoAlgoritmo::HEURISTICA, calculando))
            metodoSelecionado = MetodoAlgoritmo::HEURISTICA;
        y += 45.0f;

        // Botão de Execução
        if (calculando) {
            desenharBotao({ x, y, w, 40 }, "Calculando...", true, true);
        } else {
            if (desenharBotao({ x, y, w, 40 }, "RESOLVER AGORA", false, false)) {
                executarAlgoritmo();
            }
        }
        y += 55.0f;

        // Painel de Resultados & Métricas
        Rectangle cardRes = { x, y, w, 160.0f };
        DrawRectangleRec(cardRes, Color{ 32, 32, 48, 255 });
        DrawRectangleLinesEx(cardRes, 1.0f, Color{ 60, 60, 90, 255 });

        float ry = y + 12.0f;
        float rx = x + 15.0f;
        desenharTexto("MÉTRICAS E RESULTADOS", rx, ry, 13.0f, Color{ 140, 160, 220, 255 });
        ry += 24.0f;

        std::string tempoStr = "Tempo: " + (calculando ? "calculando..." : (resultado.tempoMs > 0 ? (std::to_string(resultado.tempoMs).substr(0, 7) + " ms") : "--"));
        desenharTexto(tempoStr, rx, ry, 15.0f, Color{ 0, 220, 130, 255 });
        ry += 24.0f;

        std::string resStr = "Resultado: " + (resultado.descricao.empty() ? "--" : resultado.descricao);
        desenharTexto(resStr, rx, ry, 13.0f, WHITE);
        ry += 22.0f;

        std::string estStr = "Grafo: " + std::to_string(grafo.estacoes.size()) + " estações, " + std::to_string(grafo.trechos.size()) + " trechos";
        desenharTexto(estStr, rx, ry, 12.0f, LIGHTGRAY);
        ry += 20.0f;

        if (problemaSelecionado == TipoProblema::MAIOR_CICLO && estacaoInicialCiclo >= 0 && estacaoInicialCiclo < (int)grafo.estacoes.size()) {
            std::string stInic = "Origem: " + grafo.estacoes[estacaoInicialCiclo].nomeLimpo;
            desenharTexto(stInic, rx, ry, 12.0f, Color{ 255, 215, 0, 255 });
        }

        // Rodapé com dicas de navegação
        float fy = (float)GetScreenHeight() - 75.0f;
        desenharTexto("NAVEGAÇÃO NO MAPA:", x, fy, 12.0f, Color{ 130, 130, 160, 255 });
        fy += 18.0f;
        desenharTexto("- Arrastar: Botão direito do mouse", x, fy, 11.0f, GRAY);
        fy += 15.0f;
        desenharTexto("- Zoom: Scroll da rodinha do mouse", x, fy, 11.0f, GRAY);
        fy += 15.0f;
        desenharTexto("- Selecionar origem: Clique na estação", x, fy, 11.0f, GRAY);
    }

    void desenharMapa() {
        BeginMode2D(camera);

        // 1. Desenha as arestas normais do metrô com as cores das linhas
        for (const auto& t : grafo.trechos) {
            Vector2 p1 = grafo.estacoes[t.u].pos;
            Vector2 p2 = grafo.estacoes[t.v].pos;
            DrawLineEx(p1, p2, 3.5f, t.cor);
        }

        // 2. Destaque do Ciclo Solução (Problema a)
        if (resultado.problema == TipoProblema::MAIOR_CICLO && resultado.ciclo.size() >= 2) {
            for (size_t i = 0; i + 1 < resultado.ciclo.size(); ++i) {
                Vector2 p1 = grafo.estacoes[resultado.ciclo[i]].pos;
                Vector2 p2 = grafo.estacoes[resultado.ciclo[i + 1]].pos;
                // Efeito de brilho duplo
                DrawLineEx(p1, p2, 8.0f, Color{ 255, 215, 0, 100 });
                DrawLineEx(p1, p2, 4.0f, Color{ 255, 235, 80, 255 });
            }
        }

        // 3. Desenha os vértices (estações)
        for (int i = 0; i < (int)grafo.estacoes.size(); ++i) {
            Vector2 p = grafo.estacoes[i].pos;
            bool ehHover = (i == hoverEstacao);
            bool ehOrigemCiclo = (i == estacaoInicialCiclo && problemaSelecionado == TipoProblema::MAIOR_CICLO);

            // Cor base do vértice
            Color corVertice = Color{ 40, 40, 60, 255 };

            // Se for coloração de manutenção, pinta com a cor atribuída
            if (resultado.problema == TipoProblema::MANUTENCAO && i < (int)resultado.cores.size() && resultado.cores[i] > 0) {
                int cIdx = (resultado.cores[i] - 1) % paletaPeriodos.size();
                corVertice = paletaPeriodos[cIdx];
            }

            // Destaque se tiver guichê (Problema b)
            bool temGuiche = false;
            if (resultado.problema == TipoProblema::GUICHES) {
                for (int g : resultado.guiches) {
                    if (g == i) { temGuiche = true; break; }
                }
            }

            if (temGuiche) {
                DrawCircleV(p, 16.0f, Color{ 255, 215, 0, 80 });
                DrawCircleLines((int)p.x, (int)p.y, 16.0f, GOLD);
            }

            if (ehOrigemCiclo) {
                DrawCircleV(p, 14.0f, Color{ 255, 100, 100, 100 });
                DrawCircleLines((int)p.x, (int)p.y, 14.0f, RED);
            }

            // Círculo principal da estação
            float raio = ehHover ? 9.0f : 6.5f;
            DrawCircleV(p, raio, corVertice);
            DrawCircleLines((int)p.x, (int)p.y, raio, WHITE);

            // Nome da estação ao lado (com fonte Ubuntu e suporte completo UTF-8)
            float tamFonte = 12.0f;
            Color corTexto = ehHover ? Color{ 255, 230, 100, 255 } : Color{ 225, 225, 245, 255 };
            desenharTexto(grafo.estacoes[i].nomeLimpo, p.x + 10.0f, p.y - 7.0f, tamFonte, corTexto);
        }

        EndMode2D();
    }

    void desenhar() {
        BeginDrawing();
        ClearBackground(Color{ 16, 16, 24, 255 });

        desenharMapa();
        desenharUI();

        // Tooltip de estação em hover
        if (hoverEstacao >= 0 && hoverEstacao < (int)grafo.estacoes.size()) {
            Vector2 m = GetMousePosition();
            const auto& est = grafo.estacoes[hoverEstacao];
            std::string info = est.nomeLimpo + " (" + std::to_string(est.vizinhos.size()) + " conexões)";
            Vector2 tw = medirTexto(info, 13.0f);
            DrawRectangle((int)m.x + 12, (int)m.y + 12, (int)tw.x + 16, 24, Color{ 10, 10, 15, 230 });
            DrawRectangleLines((int)m.x + 12, (int)m.y + 12, (int)tw.x + 16, 24, Color{ 100, 100, 140, 255 });
            desenharTexto(info, m.x + 20.0f, m.y + 16.0f, 13.0f, WHITE);
        }

        EndDrawing();
    }
};

// ============================================================================
// MAIN
// ============================================================================

int main() {
    const int larguraJanela = 1280;
    const int alturaJanela = 768;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(larguraJanela, alturaJanela, "PAA - O Metro de Paris (Grupo N=2)");
    SetTargetFPS(60);

    AppMetro app;
    app.carregarFonte();
    app.areaMapa = { 340.0f, 0.0f, (float)larguraJanela - 340.0f, (float)alturaJanela };
    app.barraLateral = { 0.0f, 0.0f, 340.0f, (float)alturaJanela };

    // Inicia carregando o dataset completo de 10 linhas
    app.carregarDataset(3);

    while (!WindowShouldClose()) {
        app.areaMapa.width = (float)GetScreenWidth() - 340.0f;
        app.areaMapa.height = (float)GetScreenHeight();
        app.barraLateral.height = (float)GetScreenHeight();

        app.atualizar();
        app.desenhar();
    }

    app.descarregarFonte();
    CloseWindow();
    return 0;
}
