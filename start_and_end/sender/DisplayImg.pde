class DisplayImg {
  PImage img;
  String imgUrl = "https://cityio.media.mit.edu/talkingdrums/image/original/";
  int pTime;
  int maxTime;


  DisplayImg() {
    img = loadImage(imgUrl, "png");
    maxTime = 3000;
  }

  void update() {
    int cTime = millis();
    if ((cTime - pTime) > maxTime) {
      img = loadImage(imgUrl, "png");

      //rest time
      pTime = cTime;
    }
  }

  void drawFullScreen() {
    if (img.width > 0 || img.height > 0 ) {
      pushMatrix();
      translate(width/2.0, height/2.0);
      rotate(PI);
      image(img, -width/2.0, -height/2.0, width, height);
      popMatrix();
    }
  }
}
