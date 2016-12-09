float div = 160;

int numX;
int numY;

void setup() {
  size(1280, 720);

  smooth(16);

  numX = int(width / div);
  numY = int(height / div);

  println(1280 / (float)div);
  println(720 / (float)div);
  println(numX+" "+numY);
}

void draw() {
  background(0);

  rectMode(CORNER);

  noStroke();
  fill(255);
  for (int i = 0; i < numX; i++) {
    for (int j = 0; j < numY; j++) {
      float x =  i * div;
      float y =  j * div;

      if ( j % 2 == 0 && i % 3 == 0) {
        fill(220);
      }else{
        fill(0);
      }
      
      if ( j % 2 == 0 && i % 4 == 1) {
        fill(155);
      }

      
      
   //   if(j % 2 == 1 && i % 3 == 1){
   //     fill(255);
   //   }
      
      
      rect(x, y, div, div);
    }
  }
}

void keyPressed() {
  if (key == 'a') {
    div+=0.1;
    println();
    println(1280 / (float)div);
    println(720 / (float)div);
    println(numX+" "+numY);
  }


  if (key == '1') {
    save(frameCount+" "+".png");
  }
}