#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <stdio.h>

#define NB_PIECES 7
#define W 10
#define H 20
#define WIN_WIDTH 500
#define WIN_HEIGTH 800

typedef struct {
  int x, y, w, h;
  char** map;
} Piece;

typedef struct {
  char map[H][W];
  int currentPiece;
  int nextPiece;
  int score;
  int lost;
  SDL_Renderer* renderer;
  SDL_Window* window;
  SDL_Texture* textures[2];
} Game;

Game g;
Piece pieces[NB_PIECES*4];

char* piecesMap[NB_PIECES*4] = {
"\
BB\
BB",

"\
L\
L\
L\
L",

"\
XX \
 XX",

"\
 V \
VVV",

"\
 GG\
GG ",

"\
D  \
DDD",

"\
  O\
OOO"
};

SDL_Rect pickAlphaNum(const char c);
SDL_Rect pickTextures(const char c);
void initPiece(int pId, int w, int h);
void initGame();
void init();
void update();
int fit();
void fall();
int randomPiece();
void slide(int);
Piece* getCurrentPiece();
Piece* getNextPiece();
char getPieceAt(Piece* p, int x, int y);
void lock();
void render();
void renderConsole();
void clearLine();
void rotateLeft();
void rotateRight();
void rotate(int);
void rotation(int p);

int main() {
  init();
  update();
  SDL_DestroyTexture(g.textures[0]);
  SDL_DestroyTexture(g.textures[1]);
  SDL_DestroyRenderer(g.renderer);
  SDL_DestroyWindow(g.window);
  return 0;
}

SDL_Rect pickAlphaNum(const char c) {
  SDL_Rect res = {0, 0, 32, 32};
  switch( c ) {
    case '#':
      res.x = 0;
      break;

    case '1':
      res.x = 32;
      break;

    case '2':
      res.x = 64;
      break;

    case '3':
      res.x = 96;
      break;

    case '4':
      res.x = 128;
      break;

    case '5':
      res.x = 160;
      break;

    case '6':
      res.x = 192;
      break;

    case '7':
      res.x = 224;
      break;

    case '8':
      res.x = 256;
      break;

    case '9':
      res.x = 288;
      break;

    case '0':
      res.x = 320;
      break;

    case ' ':
      res.x = 352;
      break;

    case 's':
      res.x = 384;
      break;

    case 'c':
      res.x = 416;
      break;

    case 'o':
      res.x = 448;
      break;

    case 'r':
      res.x = 480;
      break;

    case 'e':
      res.x = 512;
      break;

    case ':':
      res.x = 544;
      break;
  }
  return res;
}

SDL_Rect pickTextures(const char c) {
  SDL_Rect res = {0, 0, 32, 32};

  switch(c) {
    case ' ':
      res.y = 64;
      res.x = 32;
      break;

    case 'L':
      res.y = 32;
      break;

    case 'B':
      res.x = 32;
      break;

    case 'V':
      res.x = 64;
      res.y = 32;
      break;

    case 'G':
      res.x = 64;
      break;

    case 'D':
      res.x = 64;
      break;

    case 'O':
      res.y = 64;
      break;
  }

  return res;
}

void initPiece(int pId, int w, int h) {
  Piece* p = pieces + pId;

  p->x = 0;
  p->y = 0;
  p->w = w;
  p->h = h;

  p->map = piecesMap + pId;
}

void initGame() {
  g.currentPiece = randomPiece();
  g.nextPiece = randomPiece();
  g.lost = 0;
  g.score = 0;

  for(int i=0; i<H; i++)
    for(int j=0; j<W; j++)
      g.map[i][j] = ' ';

  g.window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_WIDTH, WIN_HEIGTH, SDL_WINDOW_SHOWN);
  g.renderer = SDL_CreateRenderer(g.window, -1, SDL_RENDERER_ACCELERATED);

  SDL_Surface* s = IMG_Load("blocks.png");
  g.textures[0] = SDL_CreateTextureFromSurface(g.renderer, s);
  s = IMG_Load("alpha_num.png");
  g.textures[1] = SDL_CreateTextureFromSurface(g.renderer, s);
  SDL_FreeSurface(s);
}

void init() {
  initGame();

  initPiece(0, 2, 2);
  initPiece(1, 1, 4);
  initPiece(2, 3, 2);
  initPiece(3, 3, 2);
  initPiece(4, 3, 2);
  initPiece(5, 3, 2);
  initPiece(6, 3, 2);

  for(int i=0; i<NB_PIECES; i++)
    rotation(i);
}

void update() {
  const Piece* p = getCurrentPiece();
  double fps = 1000 / 25;
  double fallSpeed = 1000 / 1;
  double fallStart = SDL_GetTicks();
  int boost = 0;
  int shift_pressed = 0;

  while( !g.lost ) {
    SDL_Event e;
    double t0 = SDL_GetTicks();

    while( SDL_PollEvent(&e) ) {
      if( e.type == SDL_QUIT )
        g.lost = 1;

      if(e.type == SDL_KEYDOWN) {
        if( e.key.keysym.sym == SDLK_LEFT && !shift_pressed)
          slide(-1);

        if( e.key.keysym.sym == SDLK_RIGHT && !shift_pressed)
          slide(1);

        if( e.key.keysym.sym == SDLK_LSHIFT)
          shift_pressed = 1;

        if( e.key.keysym.sym == SDLK_RIGHT && shift_pressed)
          rotateRight();

        if( e.key.keysym.sym == SDLK_LEFT && shift_pressed)
          rotateLeft();

        if( e.key.keysym.sym == SDLK_q )
          g.lost = 1;

        if( e.key.keysym.sym == SDLK_DOWN )
          boost = 1;
      }

      if( e.type == SDL_KEYUP ) {
        if( e.key.keysym.sym == SDLK_LSHIFT )
          shift_pressed = 0;
      }
    }
    if( boost || SDL_GetTicks() - fallStart >= fallSpeed) {
      fall();
      boost = 0;
      fallStart = SDL_GetTicks();
    }
    render();
    renderConsole();

    double delay = fps - (SDL_GetTicks() - t0);
    if(delay > 0)
      SDL_Delay(delay);
  }
}

int fit(Piece* p) {
  if(p->h + p->y > H) return 0;
  int x = p->x;
  if(p->x + p->w > W) p->x -= p->x + p->w - W;

  for(int i=0; i<p->h; i++){
    for(int j=0; j<p->w; j++){
      if( getPieceAt(p, j, i) != ' ' && g.map[p->y + i][p->x + j] != ' ') {
        p->x = x;
        return 0;
      }
    }
  }

  return 1;
}

void fall() {
  Piece* p = getCurrentPiece();

  p->y++;
  // TODO: take rotate into account
  if(!fit(p) ) {
    p->y--;
    lock();
  }
}

int randomPiece() {
  return random() % NB_PIECES;
}

void slide(int dx) {
  Piece* p = getCurrentPiece();
  p->x += dx;

  // TODO: take rotate into account
  if( p->x + p->w > W || p->x < 0 || !fit(p) )
    p->x -= dx;
}

Piece* getCurrentPiece() {
  return pieces + g.currentPiece;
}

Piece* getNextPiece() {
  return pieces + g.nextPiece;
}

char getPieceAt(Piece* p, int x, int y) {
  return (*(p->map))[y*p->w + x];
}

void lock() {
  Piece* p = getCurrentPiece();

  for(int i=0; i<p->h; i++) {
    for(int j=0; j<p->w; j++) {
      const char cell = getPieceAt(p, j, i);
      if( cell != ' ')
        g.map[i+p->y][j+p->x] = cell;
    }
  }
  clearLine();

  p->y = 0;
  p->x = 0;
  g.currentPiece = g.nextPiece;
  g.nextPiece = randomPiece();
}

void render() {
  Piece* p = getNextPiece();
  int cellSize = 25;

  SDL_RenderClear(g.renderer);

  // preview next piece
  for(int i = 0; i<p->h; i++) {
    for(int j = 0; j<p->w; j++) {
      const char cell = getPieceAt(p, j, i);
      SDL_Rect src = pickTextures(cell);
      SDL_Rect dst = {j*cellSize, i*cellSize, cellSize, cellSize};
      SDL_RenderCopy(g.renderer, g.textures[0], &src, &dst);
    }
  }

  // render score
  char* str = "score:";
  for(int i=0; i < strlen(str); i++) {
    char c = str[i];
    SDL_Rect src = pickAlphaNum(c);
    int x = WIN_WIDTH - (strlen( str ) - i)*cellSize - cellSize * 7;
    SDL_Rect dst = {x, 0, cellSize, cellSize};
    SDL_RenderCopy(g.renderer, g.textures[1], &src, &dst);
  }

  // render score
  int onRight = 0;
  for(int i=6; i>=0; i--) {
    char digit = '0' + (int)(g.score / pow(10, i)) % 10;
    if( !onRight && digit != '0')
      onRight = 1;
    SDL_Rect src = pickAlphaNum( digit );
    SDL_Rect dst = {WIN_WIDTH-cellSize*(i+1), 0, cellSize, cellSize};
    if( !onRight ) src.x = 0;
    SDL_RenderCopy(g.renderer, g.textures[1], &src, &dst);
  }

  // render gird
  p = getCurrentPiece();
  for(int i=0; i<H; i++) {
    for(int j=0; j<W; j++) {
      char cell = g.map[i][j];

      // render current piece
      if( j >= p->x && j < p->x + p->w &&
          i >= p->y && i < p->y + p->h &&
          getPieceAt(p, j-p->x, i-p->y) != ' ') {
            cell = getPieceAt(getCurrentPiece(), j-p->x, i-p->y);
      }

      SDL_Rect src = pickTextures(cell);
      SDL_Rect dst = {WIN_WIDTH/4 + j*cellSize, WIN_HEIGTH - cellSize*H + i*cellSize, cellSize, cellSize};
      SDL_RenderCopy(g.renderer, g.textures[0], &src, &dst);
    }
  }

  SDL_RenderPresent(g.renderer);
}

void renderConsole() {
  char map[H][W];
  Piece* p = getNextPiece();
  char debug_msg[500] = "";
  char consoleCanvas[W*H + W*10] = "";
  memcpy(map, g.map, sizeof(char)*H*W);
  int cellSize = 25;

  sprintf(consoleCanvas, "=== render ===\n");

  sprintf(consoleCanvas, "%sscore: %d\n\n", consoleCanvas, g.score);
  sprintf(consoleCanvas, "%spiece:\n", consoleCanvas);
  for(int i = 0; i<p->h; i++) {
    for(int j = 0; j<p->w; j++)
      sprintf(consoleCanvas, "%s%c", consoleCanvas, getPieceAt(p, j, i));
    sprintf(consoleCanvas, "%s\n", consoleCanvas);
  }

  p = getCurrentPiece();
  sprintf(consoleCanvas, "%smap\n", consoleCanvas);
  for(int i=0; i<H; i++) {
    sprintf(consoleCanvas, "%s#", consoleCanvas);
    for(int j=0; j<W; j++) {
      char cell = g.map[i][j];

      if( j >= p->x && j < p->x + p->w &&
          i >= p->y && i < p->y + p->h &&
          getPieceAt(p, j-p->x, i-p->y) != ' ') {
            cell = getPieceAt(getCurrentPiece(), j-p->x, i-p->y);

      }
      sprintf(consoleCanvas, "%s%c", consoleCanvas, cell);

    }
    sprintf(consoleCanvas, "%s#", consoleCanvas);
    sprintf(consoleCanvas, "%s%s\n", consoleCanvas, debug_msg);
    strcpy(debug_msg, "");
  }
  printf("%s", consoleCanvas);
}

void clearLine() {
  Piece* p = getCurrentPiece();
  int max = p->y + p->h;

  int bonus = 0;
  for(int i=p->y; i<max; i++) {
    int j;
    for(j=0; j<W; j++) {
      if(g.map[i][j] == ' ')
        break;
    }

    // if line is full
    if(j == W) {
      bonus = bonus * 2 + 1;

      // fall line above
      for(int k=i; k>0; k--)
        memcpy(g.map[k], g.map[k-1], sizeof(char)*W);

      // clear top line
      for(int l=0; l<W; l++)
        g.map[0][l] = ' ';
    }
  }

  g.score += bonus;
}

void rotateRight() {
  rotate(1);
}

void rotateLeft() {
  rotate(-1);
}

void rotate(int d) {
  Piece* p1 = getCurrentPiece();
  int p2Id = (g.currentPiece + d*NB_PIECES) % (NB_PIECES*4);
  if(p2Id < 0)
    p2Id += NB_PIECES*4;
  printf("id: %d\n", p2Id);
  Piece* p2 = pieces+p2Id;

  p2->x = p1->x;
  p2->y = p1->y;
  if(!fit(p2)) {
    p2->x = 0;
    p2->y = 0;
    return;
  }
  g.currentPiece = p2Id;
    
  p1->x = 0;
  p1->y = 0;
}

void rotation(int p) {
  for(int r=1; r<4; r++) {
    int id = p+NB_PIECES*r;
    initPiece(id, pieces[p].h, pieces[p].w);

    pieces[id].w = pieces[id-NB_PIECES].h;
    pieces[id].h = pieces[id-NB_PIECES].w;
    int length = pieces[p].w * pieces[p].h;

    piecesMap[id] = malloc(sizeof(char)*length);

    if( r == 1 )
      for(int i=0; i<length; i++)
        piecesMap[id][(i * 2 + 1) % (length + 1)] = piecesMap[p][i];

    else if( r == 2 )
      for(int i=0; i<length; i++)
        piecesMap[id][i] = piecesMap[p][length-i-1];

    else if ( r == 3 )
      for(int i=0; i<length; i++)
        piecesMap[id][i] = piecesMap[id-NB_PIECES*2][length-i-1];
  }
}

