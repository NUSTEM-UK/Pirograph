// Function to broadcast a PImage over UDP
// Special thanks to: http://ubaa.net/shared/processing/udp/
// This code from https://github.com/shiffman/Processing-UDP-Video-Streaming
void broadcast(PImage source, int destination) {

  PImage img = source.get();
  println(img.width, img.height); // Checks out at full res. Phew
  // Ensmallificate the image
  img.loadPixels();
  img.resize(640, 0); // proportional resize
  println(img.width, img.height);
  // We need a buffered image to do the JPG encoding
  BufferedImage bimg = new BufferedImage( img.width,img.height, BufferedImage.TYPE_INT_RGB );

  // Transfer pixels from localFrame to the BufferedImage
  bimg.setRGB( 0, 0, img.width, img.height, img.pixels, 0, img.width);

  // Need these output streams to get image as bytes for UDP communication
  ByteArrayOutputStream baStream	= new ByteArrayOutputStream();
  BufferedOutputStream bos		= new BufferedOutputStream(baStream);

  // Turn the BufferedImage into a JPG and put it in the BufferedOutputStream
  // Requires try/catch
  try {
    ImageIO.write(bimg, "jpg", bos);
  } 
  catch (IOException e) {
    e.printStackTrace();
  }

  // Get the byte array, which we will send out via UDP!
  byte[] packet = baStream.toByteArray();

  // Send JPEG data as a datagram
  println("Sending datagram with " + packet.length + " bytes");
  try {
      switch(destination) {
          case 0:
            ds0.send(new DatagramPacket(packet,packet.length, InetAddress.getByName(streamTargets[0]), clientPorts[0]));
            break;
          case 1:
            ds1.send(new DatagramPacket(packet,packet.length, InetAddress.getByName(streamTargets[1]), clientPorts[1]));
            break;
          case 2:
            ds2.send(new DatagramPacket(packet,packet.length, InetAddress.getByName(streamTargets[2]), clientPorts[2]));
            break;
          case 3:
            ds3.send(new DatagramPacket(packet,packet.length, InetAddress.getByName(streamTargets[3]), clientPorts[3]));
            break;
      }
    // stub for duplicate send to local consumer for 4-up composite sketch
    //ds.send(new DatagramPacket(packet,packet.length, InetAddress.getByName("localhost"),clientPort));
    
  } 
  catch (Exception e) {
    e.printStackTrace();
  }
}


void setupDatagramSockets() {
    // DatagramSocket stuff for UDP streaming
  try {
    println("Datagram: 0");
    ds0 = new DatagramSocket();
    println("Datagram: 0 initialised");
  } catch (SocketException e) {
    e.printStackTrace();
  } 
  try {
    println("Datagram: 1");
    ds1 = new DatagramSocket();
    println("Datagram: 1 initialised");
  } catch (SocketException e) {
    e.printStackTrace();
  }
  try {
    println("Datagram: 2");
    ds2 = new DatagramSocket();
    println("Datagram: 2 initialised");
  } catch (SocketException e) {
    e.printStackTrace();
  } 
  try {
    println("Datagram: 3");
    ds3 = new DatagramSocket();
    println("Datagram: 3 initialised");
  } catch (SocketException e) {
    e.printStackTrace();
  } 
  // For some reason, I can't get this to work via an array of DatagramSockets. Weird.
  //   for (int i = 0; i < NUMPORTS; i++) {
  //     try {
  //       println("Datagram: ", i);
  //       ds[i] = new DatagramSocket();
  //       println("Datagram: ", i, " initialised");
  //     } catch (SocketException e) {
  //       e.printStackTrace();
  //     }
  // }
}