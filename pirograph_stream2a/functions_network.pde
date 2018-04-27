// Function to broadcast a PImage over UDP
// Special thanks to: http://ubaa.net/shared/processing/udp/
// This code from https://github.com/shiffman/Processing-UDP-Video-Streaming
void broadcast(PImage source, int destination) {

  PImage img = source.get();
  // println(img.width, img.height); // Checks out at full res. Phew
  // Ensmallificate the image
  img.loadPixels();
  // 960px wide is a bit optimistic for this, but we'll give it a go.
  img.resize(960, 0); // proportional resize
  // println(img.width, img.height);
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
  // println("Sending datagram with " + packet.length + " bytes");
  try {
      switch(destination) {
          case 0:
            ds0.send(new DatagramPacket(packet,packet.length, InetAddress.getByName(streamTargets[0]), clientPorts[0]));
            ds0.send(new DatagramPacket(packet,packet.length, InetAddress.getByName("localhost"),clientPorts[destination]));
            break;
          case 1:
            ds1.send(new DatagramPacket(packet,packet.length, InetAddress.getByName(streamTargets[1]), clientPorts[1]));
            ds1.send(new DatagramPacket(packet,packet.length, InetAddress.getByName("localhost"),clientPorts[destination]));
            break;
          case 2:
            ds2.send(new DatagramPacket(packet,packet.length, InetAddress.getByName(streamTargets[2]), clientPorts[2]));
            ds2.send(new DatagramPacket(packet,packet.length, InetAddress.getByName("localhost"),clientPorts[destination]));
            break;
          case 3:
            ds3.send(new DatagramPacket(packet,packet.length, InetAddress.getByName(streamTargets[3]), clientPorts[3]));
            ds3.send(new DatagramPacket(packet,packet.length, InetAddress.getByName("localhost"),clientPorts[destination]));
            break;
      }
    // stub for duplicate send to local consumer for 4-up composite sketch
    // ds.send(new DatagramPacket(packet,packet.length, InetAddress.getByName("localhost"),clientPorts[destination]));
    
  } 
  catch (Exception e) {
    e.printStackTrace();
  }
}


void setupDatagramSockets() {
  // DatagramSocket stuff for UDP streaming
  switch(THISPORT) {
    case 0:
      try {
        println("Datagram: 0");
        ds0 = new DatagramSocket();
        println("Datagram: 0 initialised");
      } catch (SocketException e) {
        e.printStackTrace();
      }
      break;
    case 1:
      try {
        println("Datagram: 1");
        ds1 = new DatagramSocket();
        println("Datagram: 1 initialised");
      } catch (SocketException e) {
        e.printStackTrace();
      }
      break;
    case 2:
      try {
        println("Datagram: 2");
        ds2 = new DatagramSocket();
        println("Datagram: 2 initialised");
      } catch (SocketException e) {
        e.printStackTrace();
      }
      break;
    case 3:
      try {
        println("Datagram: 3");
        ds3 = new DatagramSocket();
        println("Datagram: 3 initialised");
      } catch (SocketException e) {
        e.printStackTrace();
      }
      break;
  }
}


// Handler for MQTT messages -- pulled from Heart of Maker Faire code
// I really should be passing JSON-formatted data here, but 
// this is a quick and dirty hack the evening before Maker Faire, 
// so I'm leaning on older code that worked previously.

void messageReceived(String topic, byte[] payload) {
    println("new message: " + topic + " : " + new String(payload));

    // Tokenise the topic string by splitting it on '/'
    String[] topicParts = topic.split("/");
    // Convert the payload to a String. We're not overly-worried about performance,
    // and this is easy. We may revisit later, however.
    String payloadString = new String(payload);
    String command = topicParts[1];

    // Parse commands
    // Handle channel reset
    if (command.equals("reset")) {
      if (payloadString.equals("0") && THISPORT == 0) {
        println("*** RESET ***");
        background(0);
      } else if (payloadString.equals("1") && THISPORT == 1) {
        println("*** RESET ***");
        background(0);
      } else if (payloadString.equals("2") && THISPORT == 2) {
        println("*** RESET ***");
        background(0);
      } else if (payloadString.equals("3") && THISPORT == 3) {
        println("*** RESET ***");
        background(0);
      }
    }

    // Handle save commands
    if (command.equals("save")) {
      if (payloadString.equals("0") && THISPORT == 0) {
        filename = saveFilePath + "Pirograph-A-";
        filename += year()+"-"+month()+"-"+day()+"-"+hour()+"-"+minute()+"-"+"second.png";
        composites[THISPORT].save(filename);
      } else if (payloadString.equals("1") && THISPORT == 1) {
        filename = saveFilePath + "Pirograph-B-";
        filename += year()+"-"+month()+"-"+day()+"-"+hour()+"-"+minute()+"-"+"second.png";
        composites[THISPORT].save(filename);
      } else if (payloadString.equals("2") && THISPORT == 2) {
        filename = saveFilePath + "Pirograph-C-";
        filename += year()+"-"+month()+"-"+day()+"-"+hour()+"-"+minute()+"-"+"second.png";
        composites[THISPORT].save(filename);
      } else if (payloadString.equals("3") && THISPORT == 3) {
        filename = saveFilePath + "Pirograph-D-";
        filename += year()+"-"+month()+"-"+day()+"-"+hour()+"-"+minute()+"-"+"second.png";
        composites[THISPORT].save(filename);
      }
    }
} // messageReceived