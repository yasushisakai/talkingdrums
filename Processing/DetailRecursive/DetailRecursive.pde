PImage musicImg;

void setup() {
  size(800, 800);

  musicImg = loadImage("music.png");
}

void draw() {
  background(0);

  //image(musicImg, 0, 0);

  fill(255, 50);
  for (int i = 4; i < 5; i++) {
    float nW =  width  / i;
    float nH =  height / i;

    ellipse(nW, nH, 10, 10);

    ellipse(3*nW, nH, 10, 10);

    ellipse(nW, 3*nH, 10, 10);

    ellipse(3*nW, 3*nH, 10, 10);
  }

  rectMode(CENTER);
  recursive(4, width, height);
}

void recursive(int level, float w, float h) {

  if (level == 1) {
    fill(255, 50);
    rect(w/2, h/2, w*2, h*2);
    return;
  }

  float nWidth  = w/(float)(level);
  float nHeight = h/(float)(level);

  println(nWidth+" "+nHeight+" "+level);


  fill(255, 50);
  rect(nWidth, nHeight, nWidth*2, nHeight*2);

  fill(255);
  ellipse(nWidth, nHeight, 5, 5);


  recursive(level - 1, nWidth, nHeight);
  //recursive(level - 1, 2*(level- 1)*nWidth, 2*(level- 1)*nHeight);
  //recursive(level - 1, nWidth, (level- 1)*nHeight);
  //recursive(level - 1, (level- 1)*nWidth, (level- 1)*nHeight);
}