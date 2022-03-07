#include <SDL2/SDL.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>

#define NB_PIECES 2
#define W 10
#define H 15

typedef struct {
  int x, y, w, h;
  const char** map;
} Piece;

typedef struct {
  char map[H][W];
  int currentPice;
  int score;
  int lost;
} Gird;

Gird g;
Piece pieces[NB_PIECES];

const char* piecesMap[NB_PIECES] = {
"\
XX\
XX",

"\
 X \
XXX"
};

int randomPiece();
Piece* getCurrentPiece();
char getPieceAt(Piece* p, int x, int y);
void lock();
void fall();

void initPiece(int pId, int w, int h) {
  Piece* p = pieces + pId;

  p->x = 0;
  p->y = 0;
  p->w = w;
  p->h = h;

  p->map = piecesMap + pId;
}

void initGird() {
  g.currentPice = randomPiece();
  g.lost = 0;
  g.score = 0;

  for(int i=0; i<H; i++)
    for(int j=0; j<W; j++)
      g.map[i][j] = ' ';
}

void init() {
  initGird();

  initPiece(0, 2, 2);
  initPiece(1, 3, 2);
}

void render() {
  char map[H][W];
  Piece* p = getCurrentPiece();
  char debug_msg[500] = "";

  memcpy(map, g.map, sizeof(char)*H*W);
  puts("=== render ===");
  printf("score: %d\n\n", g.score);
  puts("piece:");
  for(int i = 0; i<p->h; i++) {
    for(int j = 0; j<p->w; j++)
      putchar(getPieceAt(p, j, i));
    putchar('\n');
  }

  puts("map:");
  for(int i=0; i<H; i++) {
    putchar('#');
    for(int j=0; j<W; j++) {
      if( j >= p->x && j < p->x + p->w &&
          i >= p->y && i < p->y + p->h &&
          getPieceAt(p, j-p->x, i-p->y) != ' ') {
            strcpy(debug_msg, "<- p");
            putchar( getPieceAt(getCurrentPiece(), j-p->x, i-p->y) );
      }
      else {
        putchar( g.map[i][j] );
      }
    }
    putchar('#');
    printf(" %s\n", debug_msg);
    strcpy(debug_msg, "");
  }
}

void update() {
  const Piece* p = getCurrentPiece();

  render();
  while( !g.lost ) {
    fall();
    render();
    sleep(1);
  }
}

int fit() {
  Piece* p = getCurrentPiece();

  for(int i=0; i<p->h; i++){
    for(int j=0; j<p->w; j++){
      if( getPieceAt(p, j, i) != ' ' && g.map[p->y + i][p->x + j] != ' ')
        return 0;
    }
  }

  return 1;
}

void fall() {
  Piece* p = getCurrentPiece();

  p->y++;
  // TODO: take rotate into account
  if(p->y + p->h == H+1 || !fit() ) {
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
  if( p->x >= W || p->x < 0 || !fit() )
    p->x -= dx;
}

Piece* getCurrentPiece() {
  return pieces + g.currentPice;
}

char getPieceAt(Piece* p, int x, int y) {
  return (*(p->map))[y*p->w + x];
}

void lock() {
  Piece* p = getCurrentPiece();

  for(int i=0; i<p->h; i++) {
    for(int j=0; j<p->w; j++) {
      g.map[i+p->y][j+p->x] = getPieceAt(p, j, i);
    }
  }

  p->y = 0;
  p->x = 0;
  g.currentPice = randomPiece();
}

int main() {
  init();
  slide(-1);
  render();
  slide(1);
  slide(1);
  slide(1);
  slide(1);
  slide(1);
  slide(1);
  slide(1);
  update();
  return 0;
}
