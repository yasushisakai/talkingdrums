PImage inImage;
PImage pixImage;

int    currentColor;

//iteration for pixels
int    iterPixelX;
int    iterPixelY;

//number of pixels
int numPixelsX;
int numPixelsY;

//step division
int numberStepsX;
int numberStepsY;

boolean finishSending = false;

PGraphics pg;

void setupGraphics() {
  pg = createGraphics(width, height);
}

void setupImage() {

  //load the newest image in the directory output images
  File fp= getLatestFilefromDir(outputImageDir);
  
  String newestImage = fp.getName();
  
  if(newImage){
    String dir = outputImageDir +"/"+ newestImage;
    inImage = loadImage(dir);
    println("Image loaded "+dir);
  }else{
    inImage = loadImage(imageName);
    println("Image loaded "+imageName);
  }
  //inImage.filter(GRAY);


  inImage.updatePixels();

  iterPixelX = 0;
  iterPixelY = 0;

  //number of pixels depending on the width and the height
  numberStepsX = 1;
  numberStepsY = 1;

  numPixelsX = ceil((float)inImage.width / (float)numberStepsX);
  numPixelsY = ceil((float)inImage.height / (float)numberStepsY);
}

void drawImage() {
  image(inImage, 0, 0, width/2.0, height/2.0);
}

PImage pixeleted(int stepX, int stepY) {
  PImage pixImage = createImage(inImage.width, inImage.height, RGB);
  pixImage.updatePixels();

/*
  int counterPix = 0;
  int centerX = int(stepX / 2.0);
  int centerY  = int(stepY / 2.0);

  int interX = centerX;
  int interY = centerY;

  int numPixImageX = ceil( pixImage.width / (float) stepX );
  int numPixImageY = ceil( pixImage.height / (float) stepY );


  color col = color(0);

  //process the Image
  for (int i = 0; i < pixImage.width; i++) {
    for (int j = 0; j < pixImage.height; j++) {

      //change color depending ifon the center pixel
      if (j % interY == 0) {
        counterPix++;

        interY += stepY;
        col = inImage.get(interX, interY);
      }

      pixImage.set(i, j, col);
    }

    interY = (int)centerY;

    if (i %  stepX == 0) {
      interX += stepX;
    }
  }
  */
  return pixImage;
}

void updateIteration() {
  iterPixelX++;

  if (iterPixelX >= numPixelsX) {
    iterPixelY++;
    iterPixelX = 0;
  }

  if (iterPixelY >= numPixelsY) {
    finishSending = true;
    iterPixelX  = 0;
    iterPixelY  = 0;
    println("DONE SENDING");
  }

  println(iterPixelX+" "+iterPixelY);
}