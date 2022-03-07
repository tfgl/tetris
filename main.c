#include <SDL2/SDL.h>
#include <malloc.h>

#include <stdio.h>

#define NB_PIECES 2
#define W 5
#define H 10

typedef struct {
  int x, y, w, h;
  const char** map;
} Piece;

typedef struct {
  char map[H+1][W+1]; // -> \0
  int currentPice;
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

void initPiece(int pId, int w, int h) {
  Piece* p = pieces + pId;

  p->x = 0;
  p->y = 0;
  p->w = w;
  p->h = h;

  p->map = piecesMap + pId;
}

void initMap() {
  g.currentPice = randomPiece();
  for(int i=0; i<H; i++) {
    for(int j=0; j<W; j++)
      g.map[i][j] = ' ';
    g.map[i][W] = 0;
  }
}

void init() {
  initMap();

  initPiece(0, 2, 2);
  initPiece(1, 3, 2);
}

void render() {
  for(int i=0; i<H; i++)
    printf("#%s#\n", g.map[i]);
}

void update() {
  render();
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

  // TODO: take rotate into account
  if(p->y + p->h == H || fit() ) lock();
}

int randomPiece() {
  return 0;
}

void slide(int dx) {
  Piece* p = getCurrentPiece();
  int nextX = p->x + dx;

  // TODO: take rotate into account
  if( nextX < W && nextX >= 0 )
    p->x = nextX;
}

Piece* getCurrentPiece() {
  return pieces + g.currentPice;
}

char getPieceAt(Piece* p, int x, int y) {
  return (*(p->map))[y*p->w + x];
}

void lock() {
  Piece* p = getCurrentPiece();

  for(int i=0; i<p->h; i++)
    for(int j=0; j<p->w; j++)
      g.map[i+p->y][j+p->x] = getPieceAt(p, j, i);

  g.currentPice = random();
}

int main() {
  init();
  slide(1);
  puts("hi");
  fall();
  puts("hi");
  lock();
  puts("hi");
  render();
  puts("hi");
  return 0;
}
