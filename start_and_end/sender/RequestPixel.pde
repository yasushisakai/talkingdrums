import http.requests.*;

<<<<<<< HEAD
final String baseUrl = "https://cityio.media.mit.edu/talkingdrums/image";
final String getNextUrl = String.format("%s%s",baseUrl,"/get/next/");
final String sendDataUrl = String.format("%s%s",baseUrl,"/send/");
=======
final String baseUrl = "https://cityio.media.mit.edu/talkingdrums/image/";
final String getNextUrl = String.format("%s%s",baseUrl,"get/next/");
final String sendDataUrl = String.format("%s%s",baseUrl,"send/");
>>>>>>> 3543c2f70e4373ed7ccb5faab974a28d628ed02f

byte requestPixel () throws Exception{
  try {
    GetRequest get = new GetRequest(getNextUrl);
    get.send();
    JSONObject obj = parseJSONObject(get.getContent());
    return (byte) obj.getInt("value");
  } catch (Exception e){
    throw e;
  }
}

void sendPixel(byte d) throws Exception {
  String url = String.format("%s%d", sendDataUrl, int(d));
  try {
    // fire and forget
    GetRequest get = new GetRequest(url);
    get.send();
  } catch (Exception e){
    throw e;
  }
}

String formatByte(byte b) {
  return String.format("%d(%8s)", int(b), Integer.toBinaryString(b & 0xFF)).replace(' ','0');
}
