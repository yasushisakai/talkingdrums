class DisplayImg {
  PImage img;
  String imgUrl = "https://cityio.media.mit.edu/talkingdrums/image/current/";
  int pTime;
  int maxTime;
  boolean gotImg = false;


  DisplayImg() {
    try {
      img = loadImage(imgUrl, "png");
      gotImg = true;
    }   
    catch (Exception e) {
      println("Error: get request in image");
    }
    maxTime = 40000;
  }

  void update() {
    int cTime = millis();
    if ((cTime - pTime) > maxTime) {
      try {
        img = loadImage(imgUrl, "png");
        gotImg = true;
        println("GOT new Img");
      } 
      catch (Exception e) {
        println("Error: get request in Image");
      }

      //rest time
      pTime = cTime;
    }
  }

  void drawFullScreen() {
    try {
      if (gotImg) {
        if (img.width > 0 || img.height > 0 ) {
          image(img, 0, 0, width, height);
        }
        gotImg = false;
      }
    }
    catch (Exception e) {
      println("Error: drawing");
    }
  }
  
}
