## 1. O que precisa ser feito

1. **Montar os arquivos de dados.** Nós mesmos criamos os dois arquivos a partir do mapa real do metrô:
   - **Estações:** uma por linha, no formato `Nome x y` (coordenadas na janela, baseadas no mapa).
   - **Linhas:** seções no formato `LinhaX cor nTrechos`, seguidas dos trechos `Estacao1 Estacao2`.
   - Usamos só as linhas sorteadas pelo critério **N = (soma das matrículas) mod 5**.
   - Entram apenas as estações de **baldeação** (2 ou mais linhas) e as de **início e fim** de cada linha.
   - **Nosso N:** `___` → linhas `___________________`
2. **Interface totalmente gráfica:**
   - Desenhar o grafo: cada estação é um círculo com o nome ao lado, nas coordenadas lidas, e as arestas são retas na cor da linha.
   - Menus para resolver os 3 problemas por 3 abordagens.
3. **9 algoritmos:** 3 problemas × 3 abordagens.
   - **Força bruta.**
   - **Branch-and-bound (B&B):** construído a partir da força bruta, com um *lower/upper bound* que pode ramos improdutivos. Tem que dar o **mesmo resultado** da força bruta, e precisamos **comparar os tempos**.
   - **Heurística:** tempo **polinomial no pior caso**; não precisa garantir o ótimo. Temos que **justificar a escolha**.
4. **Testes** com arquivos de diferentes números de linhas, dentre as sorteadas: medir o tempo de cada execução e tirar prints do mapa.
5. **Relatório** em PDF, no **padrão SBC**, com **no máximo 15 páginas**.
6. **Apresentação individual:** todos precisam saber explicar e **alterar qualquer parte do código**, e pode haver prova escrita.

---

## 2. Os três problemas

Os três são problemas clássicos **NP-difíceis** em grafos. É por isso que o trabalho pede solução exata (exponencial) e aproximada (polinomial).

Grafo de exemplo usado abaixo:

```
A — B
| \ |
D — C        arestas: A-B, B-C, C-D, D-A, A-C, D-E
|
E
```

### (a) Máximo de estações com um passe → maior ciclo simples por um vértice

O turista sai de uma estação e volta a ela sem repetir nenhuma estação no caminho. Queremos o ciclo com mais estações.

- Saindo de A, o melhor é `A → B → C → D → A`, com **4 estações**.
- E nunca entra: só tem uma conexão, então chegaria lá sem ter como sair.

### (b) Guichês → conjunto dominante mínimo

Toda estação precisa ter um guichê **nela mesma ou numa vizinha**. Queremos o menor número de guichês e quais estações os recebem.

- Nenhuma estação sozinha cobre tudo: A não alcança E, e D não alcança B.
- Com `{C, D}`: C cobre A, B, C e D, e D cobre E. O mínimo é **2 guichês**.

### (c) Manutenção → coloração de grafo (número cromático)

Cada período de manutenção é uma "cor", e estações vizinhas não podem ter a mesma cor. Queremos o menor número de cores.

- A, B e C formam um triângulo, então são necessárias pelo menos 3 cores.
- Uma solução: período 1 = {A, E}, período 2 = {B, D}, período 3 = {C}. São **3 períodos**.

**Ponto de atenção:** a força bruta é exponencial, então só termina com poucas linhas. Os testes devem variar o número de linhas (por exemplo, 2, 4, 6…) justamente para mostrar isso.

---

## 3. Divisão: rodízio

Cada pessoa faz **uma abordagem diferente em cada problema**. Assim todo mundo conhece os 3 problemas para a arguição.

|              | Força bruta | Heurística | B&B |
|--------------|-------------|------------|-----|
| **Pessoa A** | (a)         | (c)        | (b) |
| **Pessoa B** | (b)         | (a)        | (c) |
| **Pessoa C** | (c)         | (b)        | (a) |

**Ordem obrigatória: força bruta → heurística → B&B.**
- O B&B parte da força bruta do mesmo problema, que é feita por outra pessoa.
- A heurística fornece a solução inicial para o B&B começar podando. Exemplo no problema (c): se a heurística acha 3 cores, o B&B já corta todo ramo que precise de 4.

---

## 4. Cronograma

| Período | Pessoa A | Pessoa B | Pessoa C |
|---|---|---|---|
| **28/10–30/10** | **Todos:** escolher a linguagem, calcular N, criar repositório **privado**, combinar a estrutura do grafo (ex.: lista de adjacência) | | |
| **28/10–01/11** | Leitura dos arquivos + estrutura do grafo | Arquivos de estações e linhas (dados) | Interface: desenho do grafo + menus vazios |
| **02/11–04/11** | Força bruta (a) | Força bruta (b) | Força bruta (c) |
| **05/11–06/11** | Heurística (c) | Heurística (a) | Heurística (b) |
| **06/11–09/11** | B&B (b) | B&B (c) | B&B (a) |
| **09/11–11/11** | **Todos:** integrar nos menus, conferir B&B = força bruta, medir tempos, testar com diferentes números de linhas, tirar prints | | |
| **10/11–14/11** | Pseudocódigo das próprias partes + **Introdução** e **Implementação** | Pseudocódigo + **Relatório de testes** | Pseudocódigo + **Conclusão** e **Bibliografia (ABNT)** |
| **14/11–15/11** | **Todos:** revisão cruzada; cada um explica e altera o código dos outros (ensaio da arguição) | | |
| **16/11 (manhã)** | **Uma única pessoa** posta o `Gnn.zip`; não deixar para perto das 19h | | |

**04/11 é prazo firme:** se a força bruta de alguém atrasar, o B&B daquele problema atrasa junto.

---

## 5. Linguagem: decidir até 30/10

O edital permite Java, C++ ou Python, com **tudo num arquivo único**.

| | Vantagens | Limitações |
|---|---|---|
| **Java (Swing)** | GUI já vem no JDK; bom desempenho; arquivo único viável | Mais verboso |
| **C++ (Qt)** | Melhor desempenho; visual mais caprichado | Qt é externo: instalação nas 3 máquinas e provável necessidade de apresentar no notebook; arquivo único só sem a macro `Q_OBJECT` |
| **Python (Tkinter)** | Tkinter embutido; escreve-se mais rápido | 30–100× mais lento; a força bruta trava com menos vértices |

**Recomendação: Java + Swing.**
- Se a opção for Qt, todos devem compilar uma janela de teste **até 30/10**. Se der problema, voltamos para Java.
- **No laboratório**, só vale o que estiver instalado oficialmente, e é preciso avisar o professor com **uma semana de antecedência**.

---

## 6. Relatório (padrão SBC, até 15 páginas)

- **a. Introdução:** problema e objetivo.
- **b. Solução proposta:** algoritmos em pseudocódigo.
- **c. Implementação:** detalhes, principalmente o que melhora a eficiência, e a organização da interface.
- **d. Relatório de testes:** testes, resultados, prints do mapa com diferentes números de linhas, tempos de execução.
- **e. Conclusão:** comparar força bruta, B&B e heurística quanto à complexidade de tempo e memória e quanto ao tempo medido.
- **f. Bibliografia:** padrão ABNT.

---

## 7. Checklist de entrega

- [ ] Arquivo `Gnn.zip` (com o número do grupo dado pelo professor), **até 10 MB**
- [ ] Código-fonte e relatório no **diretório raiz** do zip
- [ ] Nome de todos os integrantes **no início do código** e no relatório
- [ ] Relatório com **no máximo 15 páginas**
- [ ] Postado por **uma única pessoa** (senão, −5 pontos)

---

## 8. Regras importantes

- Proibido código ou documentação **gerados por IA**, cópia ou ajuda de terceiros. Isso **anula a nota**.
- Repositório no GitHub **privado**.
- **Todos** precisam comparecer à apresentação.

---

## 9. Referências úteis

- LEVITIN, A. *Introduction to the Design and Analysis of Algorithms*: capítulos de busca exaustiva e de backtracking/branch-and-bound.
- ZIVIANI, N. *Projeto de Algoritmos*: problemas NP-completos e heurísticas.
- **(a)** DIXON, E. T.; GOODMAN, S. E. An algorithm for the longest cycle problem. *Networks*, 1976. B&B para o maior ciclo que contém um vértice fixo.
- **(b)** JIANG, H.; ZHENG, Z. An Exact Algorithm for the Minimum Dominating Set Problem. *IJCAI*, 2023.
- **(c)** SAN SEGUNDO, P. A new DSATUR-based algorithm for exact vertex coloring. *Computers & Operations Research*, 2012.
- **(c)** FURINI, F.; GABREL, V.; TERNIER, I.-C. An Improved DSATUR-Based Branch-and-Bound Algorithm for the Vertex Coloring Problem. *Networks*, 2017.

### Heurísticas

- **(a)** CHALUPA, D.; BALAGHAN, P.; HAWICK, K. A.; GORDON, N. A. Computational methods for finding long simple cycles in complex networks. *Knowledge-Based Systems*, v. 125, p. 96–107, 2017.
  - Constrói um ciclo longo inicial com DFS e depois aplica busca local com perturbações.
  - **Para nós, interessa a parte da construção por DFS**, que é polinomial.
- **(b)** PAREKH, A. K. Analysis of a greedy heuristic for finding small dominating sets in graphs. *Information Processing Letters*, v. 39, p. 237–240, 1991.
  - Heurística gulosa padrão: a cada passo, escolhe o vértice que cobre mais vértices ainda não cobertos.
  - Exemplo no grafo da seção 2: o 1º passo escolhe A (cobre A, B, C, D), o 2º escolhe D (cobre E). Resultado: {A, D}, com 2 guichês.
- **(b)** SANCHIS, L. A. Experimental Analysis of Heuristic Algorithms for the Dominating Set Problem. *Algorithmica*, v. 33, p. 3–18, 2002.
  - Compara variações da gulosa; serve para justificar a escolha da heurística.
- **(c)** WELSH, D. J. A.; POWELL, M. B. An upper bound for the chromatic number of a graph and its application to timetabling problems. *The Computer Journal*, v. 10, p. 85, 1967.
  - Ordena os vértices por grau (decrescente) e dá a cada um a menor cor livre.
- **(c)** BRÉLAZ, D. New methods to color the vertices of a graph. *Communications of the ACM*, v. 22, n. 4, p. 251–256, 1979.
  - DSATUR heurístico **e** sua versão exata (B&B) no mesmo artigo.

Os artigos são bem mais sofisticados do que o trabalho exige. Servem para tirar ideias de bound e heurística e para a bibliografia. **Conferir o formato ABNT antes de colocar no relatório.**
