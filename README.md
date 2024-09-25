##### README (PT-BR)

# Guache - Desenho 2D 🎨 
#### *Por Helen Christine Turbano, João Isaac Alves e Riquelme Jatay.*
Guache é uma aplicação simples, mas robusta de pintura 2D, desenvolvida com OpenGL e GLUT. Ela permite aos usuários desenhar pontos, linhas e polígonos na tela, aplicar transformações, e salvar/carregar desenhos de arquivos. Este foi o primeiro projeto da disciplina de Computação Gráfica no curso de Ciência da Computação na Universidade Federal do Cariri.

## Funcionalidades 🚀
- **Desenhar Formas**: Crie pontos, linhas e polígonos com facilidade.
- **Transformações**: Rotacione, escale e aplique cisalhamento nas formas.
- **Seleção**: Clique para selecionar formas para manipulação.
- **Salvar/Carregar**: Salve sua arte em arquivos e carregue-os de volta para edição.
- **Animação**: Anime o polígono selecionado.

## Instalação 💻
Para começar a usar o Guache, siga os seguintes passos:

1. **Clone o Repositório**:
   ```bash
   git clone https://github.com/seuusuario/guache.git
   cd guache
   ```

2. **Instale Dependências**: Certifique-se de ter OpenGL e GLUT instalados no seu sistema.

3. **Compile o Programa**:
   ```bash
   gcc -o guache main.c -lGL -lGLU -lglut
   ```

4. **Execute o Programa**:
   ```bash
   ./guache
   ```

   Ou para carregar um projeto existente:
   ```bash
   ./guache load <nome_do_arquivo>
   ```

   Ou para salvar um projeto:
   ```bash
   ./guache save <nome_do_arquivo>
   ```

## Controles ⌨️

- **Desenhar Ponto**: Pressione `v`
- **Desenhar Linha**: Pressione `b`
- **Desenhar Polígono**: Pressione `n`
- **Selecionar Objeto**: Pressione `m`
- **Remover Objeto Selecionado**: Pressione `0`
- **Rotacionar Polígono Selecionado**: Pressione `z`
- **Mudar Cor**:
   - `1` - Preto
   - `2` - Vermelho
   - `3` - Verde
   - `4` - Azul
   - `5` - Amarelo
   - `6` - Magenta
   - `7` - Ciano

- **Transformações**:
   - **Mover**: `a` (esquerda), `s` (baixo), `d` (direita), `w` (cima)
   - **Rotacionar**: `q` (direita), `e` (esquerda)
   - **Escalar**: `+` (aumentar), `-` (diminuir)
   - **Cisalhar**: `i` (direita), `j` (cima), `k` (esquerda), `l` (baixo)
   - **Refletir**: `x` (eixo x), `y` (eixo y)

## Exemplo de Uso 💡

1. Abra a aplicação.
2. Selecione o modo que deseja desenhar (ponto, linha, polígono).
3. Clique na tela para criar suas formas.
4. Use os atalhos de teclado para aplicar transformações.
5. Salve sua arte para edições futuras.

##### README (EN)

# Guache - 2D Painter 🎨
#### *By Helen Christine Turbano, João Isaac Alves and Riquelme Jatay.*
Guache is a simple yet robust 2D painting application built using OpenGL and GLUT. It allows users to draw points, lines, and polygons on the screen, apply transformations, and save/load drawings from files. This was the first project of the Computer Graphics course in the Computer Science program at the Federal University of Cariri.

## Features 🚀

- **Draw Shapes:** Create points, lines, and polygons with ease.
- **Transformations:** Rotate, scale, and shear shapes.
- **Selection:** Click to select shapes for manipulation.
- **Save/Load:** Save your artwork to files and load them back for editing.
- **Animation:** Animate the selected polygon.

## Installation 💻

To get started with Guache, follow these steps:

1. **Clone the Repository:**
   ```bash
   git clone https://github.com/yourusername/guache.git
   cd guache


2.  **Install Dependencies:** Make sure you have OpenGL and GLUT installed on your system.
    
3.  **Compile the Program:**
    
    
    ```bash
    gcc -o guache main.c -lGL -lGLU -lglut
    ```
    
4.  **Run the Program:**
    
    ```bash
    ./guache
    ```
    
    Or to load an existing project:
    
    
    ```bash
    ./guache load <filename>
    ```
    
    Or to save a project:
    
    
    ```bash
    ./guache save <filename>
    ```
    

## Controls ⌨️

-   **Draw Point:** Press `v`
-   **Draw Line:** Press `b`
-   **Draw Polygon:** Press `n`
-   **Select Object:** Press `m`
-   **Remove Selected Object:** Press `0`
-   **Rotate Selected Polygon:** Press `z`
-   **Change Color:**
    -   `1` - Black
    -   `2` - Red
    -   `3` - Green
    -   `4` - Blue
    -   `5` - Yellow
    -   `6` - Magenta
    -   `7` - Cyan
-   **Transformations:**
    -   Move: `a` (left), `s` (down), `d` (right), `w` (up)
    -   Rotate: `q` (right), `e` (left)
    -   Scale: `+` (increase), `-` (decrease)
    -   Shear: `i` (right), `j` (up), `k` (left), `l` (down)
    -   Reflect: `x` (x-axis), `y` (y-axis)

## Usage Example 💡

1.  Open the application.
2.  Select the mode you want to draw in (point, line, polygon).
3.  Click on the canvas to create your shapes.
4.  Use the keyboard shortcuts to apply transformations.
5.  Save your artwork for future editing.
