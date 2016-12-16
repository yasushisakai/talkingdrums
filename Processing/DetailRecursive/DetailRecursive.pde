PImage musicImg;

void setup() {
  size(800, 800);

  musicImg = loadImage("music.png");
}

void draw() {
  background(0);

  //image(musicImg, 0, 0);

  rectMode(CENTER);

  /*
  fill(255, 20);
   recursiveRect(width/8, 10, 10, width - 20, height - 20 );
   */

  fill(255, 20);
  recursiveRect(width/16, 10, 10, width - 20, height - 20 );

  fill(255, 80);
  recursiveRect(width/4, 10, 10, width - 20, height - 20 );
}

void recursiveRect(int level, float x, float y, float w, float h) {

  float subW = w/2.0;
  float subH = h/2.0;

  rect(x + subW, y + subW, w, h);

  if ( subW >= level || subH >= level ) {

    recursiveRect(level, x, y, subW, subH);
    recursiveRect(level, x + subW, y, subW, subH);
    recursiveRect(level, x, y + subH, subW, subH);
    recursiveRect(level, x + subW, y + subH, subW, subH);

    // recursiveRect(x,   y, subW, subH);
  }
}

void recursiveRect(int level, int pos, float x, float y, float w, float h) {

  float subW = w/2.0;
  float subH = h/2.0;

  rect(x + subW, y + subW, w, h);

  if ( subW >= level || subH >= level ) {

    if (pos == 1) {
      recursiveRect(level, x, y, subW, subH);
    } else if (pos == 2) {
      recursiveRect(level, x + subW, y, subW, subH);
    } else if (pos == 3) {
      recursiveRect(level, x, y + subH, subW, subH);
    } else if (pos == 4) {
      recursiveRect(level, x + subW, y + subH, subW, subH);
    }

    // recursiveRect(x,   y, subW, subH);
  }
}