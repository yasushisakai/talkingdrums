void keyPressed(){
  if(key  == 'a'){
   updateIteration(); 
  }
  
  if(key == '2'){
   println(System.getProperty("user.dir"));
   
   Path currentRelativePath = Paths.get("");
   String s = currentRelativePath.toAbsolutePath().toString();
   println(s);
  }
  
  if(key == '3'){
    //print(sketchPath("")+"../output_images/");
  }
  
  if(key == '1'){
   //String output_image_dir = sketchPath("")+"../output_images/";
   File fp= getLatestFilefromDir(outputImageDir);
   println(fp.getName());
  }
  
  if(key == 'z'){
   requestImage(); 
  }
  
 
  
}