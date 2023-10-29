package Offline1.Server;

import java.io.*;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.*;

public class ClientHandler{
    private Server fileServer;
    private Socket socket1;
    private Socket socket2;
    private Socket socket3;

    protected ObjectOutputStream out;
    protected ObjectInputStream in;
    protected DataOutputStream fileOut;
    protected DataInputStream fileIn;
    protected ObjectOutputStream broadcastOut;
    protected FileOutputStream fileOutputStream;

    protected String username;
    protected boolean isLogIn;

    public ClientHandler(Socket socket1,Socket socket2,Socket socket3, Server fileServer) {
        this.socket1 = socket1;
        this.socket2 = socket2;
        this.socket3 = socket3;
        this.fileServer = fileServer;
        try {
            out = new ObjectOutputStream(this.socket1.getOutputStream());
            in = new ObjectInputStream(this.socket1.getInputStream());
            fileOut = new DataOutputStream(this.socket2.getOutputStream());
            fileIn = new DataInputStream(this.socket2.getInputStream());
            broadcastOut = new ObjectOutputStream(this.socket3.getOutputStream());

        }catch (Exception e){
            System.out.println("Failed to connect with client");
        }

        Thread othersThread = new Thread(new Runnable() {

            @Override
            public void run() {
                try {
                    username = (String) in.readObject();
                    boolean success = login(username);
                    if(success){
                        out.writeObject("SUCCESS: Connected to the server.");
                    }
                    else{
                        out.writeObject("ERROR: Username is already in use.");
                        socket1.close();
                        socket2.close();
                        socket3.close();
                        return;
                    }


                    while (true) {
                        String command = (String) in.readObject();
                        switch (command) {
                            case "OTHERS-FILE-LIST":
                                Map<String,List<String>> clientFiles = new HashMap<>();
                                for(Map.Entry<String,ClientHandler> clientEntry: fileServer.clientHandlers.entrySet()){
                                    String client = clientEntry.getKey();
                                    if(!client.equalsIgnoreCase(username)){
                                        String pubDirPath = "serverStorage/"+client+"/public";
                                        List<String> publicFileName = new ArrayList<>();
                                        File directory = new File(pubDirPath);
                                        File[] files = directory.listFiles();
                                        if (files != null) {
                                            for (File file : files) {
                                                if (file.isFile()) {
                                                    publicFileName.add(file.getName());
                                                }
                                            }
                                        }
                                        clientFiles.put(client,publicFileName);
                                    }

                                }
                                out.writeObject(clientFiles);
                                break;
                            case "OWN-FILE-LIST":
                                String pubDirPath = "serverStorage/"+username+"/public";
                                List<String> publicFileName = new ArrayList<>();
                                File directory = new File(pubDirPath);
                                File[] files = directory.listFiles();
                                if (files != null) {
                                    for (File file : files) {
                                        if (file.isFile()) {
                                            publicFileName.add(file.getName());
                                        }
                                    }
                                }
                                String prtDirPath = "serverStorage/"+username+"/private";
                                List<String> privateFileName = new ArrayList<>();
                                directory = new File(prtDirPath);
                                files = directory.listFiles();
                                if (files != null) {
                                    for (File file : files) {
                                        if (file.isFile()) {
                                            privateFileName.add(file.getName());
                                        }
                                    }
                                }
                                Map<String,List<String>> list = new HashMap<>();
                                list.put("public",publicFileName);
                                list.put("private",privateFileName);
                                out.writeObject(list);
                                break;
                            case "CLIENT-LIST":
                                Map<String, Boolean> clients = new HashMap<>();
                                for(Map.Entry<String,ClientHandler> clientEntry: fileServer.clientHandlers.entrySet()) {
                                    String client = clientEntry.getKey();
                                    ClientHandler handler = clientEntry.getValue();
                                    if(handler.isLogIn)
                                        clients.put(client,true);
                                    else
                                        clients.put(client,false);
                                }
                                out.writeObject(clients);
                                break;

                            case "REQUEST":
                                String desc = (String) in.readObject();
                                String rid = fileServer.addNewFileRequest(username,desc);
                                out.writeObject("Request acknowledged. Request ID: "+rid);
                                break;
                            case "MESSAGES":
                                List<String> msgList = fileServer.getMessage(username);
                                out.writeObject(msgList);
                                break;
                            case "LOGOUT":
                                System.out.println(username+": logout");
                                isLogIn = false;
                                socket1.close();
                                socket2.close();
                                socket3.close();
                                return;
                            default:
                                out.writeObject("ERROR: Invalid command.");
                                break;
                        }
                    }

                }catch (Exception e) {
                    try {
                        isLogIn = false;
                        socket1.close();
                        socket2.close();
                        socket3.close();
                    } catch (Exception e1) {
                        //e.printStackTrace();
                    }
                    System.out.println(username+": disconnected");
                }
            }
        });

        othersThread.start();

        Thread fileTransferThread = new Thread(new Runnable() {

            @Override
            public void run() {
                try {
                    while (true) {
                        String command =  fileIn.readUTF();

                        switch (command) {
                            case "UPLOAD":
                                System.out.println(username +": upload request");
                                String fileName = fileIn.readUTF();
                                if(fileName.equalsIgnoreCase("terminated"))
                                    continue;
                                String filePath = null;
                                long fileSize=0;
                                try {
                                    fileSize =  fileIn.readLong();
                                    String acc = fileIn.readUTF();

                                    Boolean isReq = false;
                                    String rid = null;
                                    if(acc.equalsIgnoreCase("public") || acc.equalsIgnoreCase("private")){
                                        filePath = "serverStorage/"+username+"/"+acc+"/"+fileName;
                                        fileOutputStream = new FileOutputStream(filePath);
                                    }

                                    else{
                                        isReq=true;
                                        rid = fileIn.readUTF();
                                        filePath = "serverStorage/"+username+"/public/"+fileName;
                                        fileOutputStream = new FileOutputStream(filePath);
                                    }
                                    System.out.println(username+": "+fileIn.readUTF());
                                    fileOut.writeUTF("UPLOAD");
                                    if(fileServer.usedBufferSize+fileSize>fileServer.MAX_BUFFER_SIZE){
                                        fileOut.writeUTF("Failed: Can not take the request right now. Buffer is full");
                                    }
                                    else {
                                        int fid = fileServer.fileID;
                                        fileServer.fileID += 1;

                                        fileOut.writeUTF("f-" + Integer.toString(fid));
                                        int chunkSize = fileServer.genRandomChunkSize();
                                        byte[] buffer = new byte[chunkSize];
                                        fileServer.usedBufferSize += fileSize;
                                        fileOut.writeInt(chunkSize);
                                        int bytesRead;
                                        long totalReceived = 0;
                                        int flag = 0;
                                        while ((bytesRead = fileIn.read(buffer)) != -1) {
                                            String data = new String(buffer, bytesRead - 9, 9);
                                            if (data.equalsIgnoreCase("  Timeout")) {
                                                flag = -1;
                                                break;
                                            } else if (data.equalsIgnoreCase("Completed")) {
                                                flag = 1;
                                                break;
                                            } else {
                                                totalReceived += bytesRead;
                                                System.out.println(username + ": Received " + bytesRead + " bytes, total " + totalReceived + " bytes");
                                                fileOut.writeUTF("ACK: Received " + bytesRead + " bytes, total " + totalReceived + " bytes");
                                                fileOutputStream.write(buffer, 0, bytesRead);
                                                fileOut.flush();
                                                Thread.sleep(1000);
                                            }
                                        }
                                        fileOutputStream.close();
                                        fileServer.usedBufferSize -= fileSize;
                                        if (flag == 1 && fileSize == totalReceived) {
                                            if (isReq) {
                                                FileRequest file = fileServer.requestedFiles.get(rid);
                                                String reqClient = file.client;
                                                List<Message> reqClientMsg = fileServer.clientMessages.get(reqClient);
                                                Message msg = new Message(file, username);
                                                reqClientMsg.add(msg);
                                                fileServer.clientMessages.put(reqClient, reqClientMsg);
                                            }
                                            System.out.println(username + ": upload completed, total received: " + totalReceived);
                                            fileOut.writeUTF("File Received");
                                        } else {
                                            File file = new File(filePath);
                                            file.delete();
                                            System.out.println(username + ": Transfer terminated");
                                            fileOut.writeUTF("Termination successful");
                                        }


                                    }
                                }catch (Exception e){
                                    fileServer.usedBufferSize -= fileSize;

                                    try {
                                        isLogIn = false;
                                        fileOutputStream.close();
                                        socket1.close();
                                        socket2.close();
                                        socket3.close();
                                        File file = new File(filePath);
                                        file.delete();
                                    } catch (IOException e1){

                                    }
                                    System.out.println(username+": disconnected");
                                }
                                break;
                            case "DOWNLOAD":
                                System.out.println(username+": download request");
                                String whos = fileIn.readUTF();
                                String dfileName;
                                String dPath;
                                if(whos.equalsIgnoreCase("OWN")){
                                    String acc = fileIn.readUTF();
                                    dfileName = fileIn.readUTF();
                                    dPath = "serverStorage/"+username+"/"+acc+"/"+dfileName;
                                }
                                else{
                                    String clientName = fileIn.readUTF();
                                    dfileName = fileIn.readUTF();
                                    dPath = "serverStorage/"+clientName+"/public/"+dfileName;
                                }
                                System.out.println(username+": "+fileIn.readUTF());
                                fileOut.writeUTF("DOWNLOAD");
                                fileOut.writeInt(fileServer.MAX_BUFFER_SIZE);
                                File file = new File(dPath);
                                if (file.exists()) {
                                    FileInputStream fileInputStream = new FileInputStream(file);
                                    int bytesRead;
                                    byte[] buffer = new byte[fileServer.MAX_BUFFER_SIZE];
                                    while ((bytesRead = fileInputStream.read(buffer)) != -1){
                                        fileOut.write(buffer, 0, bytesRead);
                                        fileOut.flush();
                                        System.out.println(username+": sent "+bytesRead+" bytes");
                                    }
                                    System.out.println(username+": File sent");
                                    fileOut.writeUTF("File Sent");
                                    fileInputStream.close();
                                } else {
                                    System.out.println("File not found!");
                                }
                                break;
                            default:
                                fileOut.writeUTF("ERROR: Invalid command.");
                                break;
                        }
                    }
                }catch (Exception e){
                    try {
                        isLogIn = false;
                        socket1.close();
                        socket2.close();
                        socket3.close();
                    } catch (IOException e1) {

                    }
                    System.out.println(username+": disconnected");
                }
            }
        });
        fileTransferThread.start();
    }

    protected synchronized boolean login(String username) {

        if(fileServer.clientHandlers.containsKey(username)){
            ClientHandler handler = fileServer.clientHandlers.get(username);
            if(handler.isLogIn){
                return false;
            }
            System.out.println(username+": log in.");
        }
        else {
            System.out.println(username+": Sign Up and Log in");
            String dirPath = "serverStorage/"+username;
            File directory = new File(dirPath);
            if (directory.exists()) {
                directory.delete();
            }
            boolean created = directory.mkdirs();
            File pubDir = new File(dirPath+"/public");
            boolean created1 = pubDir.mkdirs();
            File priDir = new File(dirPath+"/private");
            boolean created2 = priDir.mkdirs();
            fileServer.clientMessages.put(username, new ArrayList<>());
        }

        fileServer.clientHandlers.put(username, this);
        isLogIn = true;
        return true;
    }
    protected void broadcast(String msg){
        try {
            broadcastOut.writeObject(msg);
        }catch (Exception e){
            e.printStackTrace();
        }
    }

}
