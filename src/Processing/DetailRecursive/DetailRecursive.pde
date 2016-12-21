PImage musicImg;

ArrayList generateSquaresPos;
ArrayList generateSquaresSizes;

ArrayList generateSquareActive;

int incrementPos = 0;

void setup() {
  size(800, 800);

  generateSquaresPos   = new ArrayList<PVector>();
  generateSquaresSizes = new ArrayList<PVector>();
  generateSquareActive = new ArrayList<Integer>();

  musicImg = loadImage("music.png");

  recursiveRect(width/(32), 10, 10, width - 20, height - 20 );

  for (int i = 0; i <generateSquaresPos.size(); i++) {
    generateSquareActive.add(0);
  }
}

void draw() {
  background(0);

  //image(musicImg, 0, 0);

  rectMode(CENTER);


  for (int  i = 0; i < incrementPos; i++) {
    PVector pos = (PVector)generateSquaresPos.get(i);
    PVector tam = (PVector)generateSquaresSizes.get(i);

    int activation = (Integer)generateSquareActive.get(i);

    color centerColor = musicImg.get( (int)pos.x, (int) pos.y);

    fill(centerColor);
    rect(pos.x, pos.y, tam.x, tam.y);
  }

  /* 
   
   for (int  i = 0; i < generateSquaresPos.size(); i++) {
   PVector pos = (PVector)generateSquaresPos.get(i);
   PVector tam = (PVector)generateSquaresSizes.get(i);
   
   int activation = (Integer)generateSquareActive.get(i);
   
   color centerColor = musicImg.get( (int)pos.x, (int) pos.y);
   
   if (activation == 1) {
   fill(centerColor);
   rect(pos.x, pos.y, tam.x, tam.y);
   }
   
   }
   
   */

  /*
  fill(255, 20);
   recursiveRect(width/8, 10, 10, width - 20, height - 20 );
   
   
   fill(255, 20);
   recursiveRect(width/16, 10, 10, width - 20, height - 20 );
   
   fill(255, 80);
   recursiveRect(width/4, 10, 10, width - 20, height - 20 );
   */
}

void keyPressed() {
  if (key == 'a') {
    incrementPos++;

    if (incrementPos >= generateSquaresPos.size() ) {
      incrementPos = 0;
    }
  }

  if (key == 's') {
    int randomPos = int(random(0, generateSquaresPos.size()));
    generateSquareActive.set(randomPos, 1);
    println(randomPos);
    println(generateSquareActive.get(randomPos));
  }
}

void recursiveRect(int level, float x, float y, float w, float h) {

  float subW = w/2.0;
  float subH = h/2.0;

  rect(x + subW, y + subW, w, h);
  generateSquaresPos.add(new PVector(x + subW, y + subW));
  generateSquaresSizes.add(new PVector(w, h));

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