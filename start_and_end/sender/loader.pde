PImage loadImageWithFallback (String whichImage) throws Exception {

  PImage img;

  try {
    img = loadImage(baseUrl + whichImage + "/");
  } catch (Exception e) {
    // second attempt to load from local
    try {
      img = loadImage(whichImage + ".png");
    } catch (Exception e) {
      throw e;
    }
  }

  if(img.width < 0 || img.height < 0){
    try {
      img = loadImage(whichImage + ".png");
    } catch (Exception e) {
      throw e;
    }
  }

  img.save(whichImage + ".png");

  return img;

}

// PImage img = loadCurrentImage()
// img.draw(10, 10);


final String statusUrl = "https://cityio.media.mit.edu/talkingdrums/image/status/";

void getStatus () throws Exception {

  try{
    GetRequest get = new GetRequest(statusUrl);
    get.send();
  } catch (Exception e){
    println("couldn't get request");
    return;
  }

  try{
    JSONObject status = parseJSONObject(get.getContent());
  }catch (Exception e){
    println("couldn't parse json");
    return;
  }

  // is_running:bool
  // last_update: SystemTime {secs_since_epoch}
  // width
  // height
  // cnt_in
  // cnt_out
  // image_bytes...(do we need them? I don't know)

}


// assuming colors are each 16 bits
PVector[] cntToXY (int cnt, int h) {
  // again 16 * 3 = 48 bytes
  PVector[] pos = new PVector[2];
  if(cnt < 48){
    return null;
  }
  int index = cnt - 48; 
  int x = index * 2 / h;
  int y = index * 2 % h;
  pos[0] = new PVector(x, y);
  int x = (index * 2 + 1) / h;
  int y = (index * 2 + 1) % h;
  pos[1] = new PVector(x, y);
  return pos;
}



