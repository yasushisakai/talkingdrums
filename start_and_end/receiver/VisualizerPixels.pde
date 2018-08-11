class PixelImage {  
  //default img
  int widthImg  =150;
  int heightImg = 100;

  //pixel image
  float pixelTamX;
  float pixelTamY;

  //iterator
  int itrX;
  int itrY;

  color pix;

  PImage checkImg;

  PixelImage() {
    pixelTamX = width/(float)widthImg;
    pixelTamY =  height/(float)heightImg;

    itrX = 0;
    itrY = 0;

    println(pixelTamX+" "+pixelTamY);

    checkImg = loadImage( "current.png");
  }


  void draw() {
    float posx = itrX * pixelTamX;
    float posy = itrY * pixelTamY; 

    noStroke();
    fill(pix);
    rect(posx, posy, pixelTamX, pixelTamY);

    println(posx+ " "+posy);
  }

  void update(int pixel) {
    //color checkColor  = checkImg.get(itrX, itrY);
    pix = color(pixel);
    
    itrX++;
    if (itrX >= widthImg) {
      itrX=0;
      itrY++;
    }


    if ( itrY >= heightImg) {
      println("done receiving img");
    }

    println(itrX+" "+itrY);
  }
}
