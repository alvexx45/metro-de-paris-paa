# ==============================================================================
# Makefile para o Trabalho Prático de PAA - Metrô de Paris
# ICEI - PUC Minas | Grupo N=2
# ==============================================================================

CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SRC_DIR = src
BIN_DIR = .
TARGET = $(BIN_DIR)/metro_paris
SRC = $(SRC_DIR)/main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	@echo "Compilando $(TARGET)..."
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)
	@echo "Compilação concluída com sucesso! Execute com ./metro_paris ou 'make run'"

run: $(TARGET)
	./metro_paris

clean:
	@echo "Limpando arquivos gerados..."
	rm -f $(TARGET) *.zip

# Gera o arquivo zip no formato exigido pelo edital (código no diretório raiz)
package:
	@echo "Preparando pacote de entrega para o Canvas..."
	@cp $(SRC_DIR)/main.cpp ./main.cpp
	@zip -r entrega_metro_paris.zip main.cpp testes_N2/ assets/ docs/
	@rm -f ./main.cpp
	@echo "Pacote 'entrega_metro_paris.zip' gerado com sucesso!"

help:
	@echo "Comandos disponíveis:"
	@echo "  make         - Compila o executável principal ($(TARGET))"
	@echo "  make run     - Compila e executa o programa"
	@echo "  make clean   - Remove os binários gerados"
	@echo "  make package - Cria o arquivo ZIP estruturado para entrega"
	@echo ""
	@echo "Pré-requisitos da Raylib:"
	@echo "  Arch Linux:    sudo pacman -S raylib"
	@echo "  Ubuntu/Debian:  sudo apt install libraylib-dev libgl1-mesa-dev libx11-dev"
	@echo "  Fedora:        sudo dnf install raylib-devel"

.PHONY: all run clean package help
