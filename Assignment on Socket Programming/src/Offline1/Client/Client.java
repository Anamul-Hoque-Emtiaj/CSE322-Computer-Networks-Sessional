package Offline1.Client;

import java.io.*;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.util.List;
import java.util.Map;
import java.util.Scanner;

public class Client {
    private static final String SERVER_IP = "localhost";
    private static final int SERVER_PORT1 = 6666;
    private static final int SERVER_PORT2 = 6667;
    private static final int SERVER_PORT3 = 6668;
    private static Scanner scanner;


    private Socket socket1;
    private Socket socket2;
    private Socket socket3;

    private String username;
    private ObjectOutputStream out;
    private ObjectInputStream in;
    private DataOutputStream fileOut;
    private DataInputStream fileIn;
    private ObjectInputStream broadcastIn;
    private Thread commandThread,fileTransferThread,broadcastThread;
    private FileInputStream fileInputStream;
    private FileOutputStream fileOutputStream;
    private boolean fileTransfer;


    private void print(){
        System.out.println("\n------Option------");
        System.out.println("Press 1 to see clients list");
        System.out.println("Press 2 to see your files");
        System.out.println("Press 3 to see other's public files");
        System.out.println("Press 4 to see your unread message");
        System.out.println("Press 5 to upload file");
        System.out.println("Press 6 to send request for a file");
        System.out.println("Press 7 to logout");
    }

    Client(){
        try {
            socket1 = new Socket(SERVER_IP, SERVER_PORT1);
            socket2 = new Socket(SERVER_IP, SERVER_PORT2);
            socket3 = new Socket(SERVER_IP, SERVER_PORT3);

            out = new ObjectOutputStream(socket1.getOutputStream());
            in = new ObjectInputStream(socket1.getInputStream());


            fileOut = new DataOutputStream(socket2.getOutputStream());
            fileIn = new DataInputStream(socket2.getInputStream());

            broadcastIn = new ObjectInputStream(socket3.getInputStream());
            fileTransfer = false;
        }catch (Exception e){
            e.printStackTrace();
        }
        commandThread = new Thread(new Runnable() {

            @Override
            public void run() {

                try {
                    System.out.print("Enter your username: ");
                    username = scanner.nextLine();
                    out.writeObject(username);
                    String res = (String) in.readObject();
                    System.out.println(res);
                    if (!res.startsWith("SUCCESS")) {
                        socket1.close();
                        socket2.close();
                        socket3.close();
                        return;
                    }
                    else {
                        String dirPath = "clientStorage/"+username;
                        File directory = new File(dirPath);
                        if(!directory.exists()){
                            boolean created = directory.mkdirs();
                        }

                        while (true){
                            print();
                            String option = scanner.nextLine();
                            if(option.equalsIgnoreCase("1")){
                                out.writeObject("CLIENT-LIST");
                                Map<String, Boolean> clients = (Map<String, Boolean>) in.readObject();
                                System.out.println("Client's Name  ----  IsActiveNow");
                                for(Map.Entry<String, Boolean> client: clients.entrySet()){
                                    System.out.println(client.getKey()+"  ----  "+client.getValue());
                                }
                            }
                            else if(option.equalsIgnoreCase("2")){
                                out.writeObject("OWN-FILE-LIST");
                                Map<String,List<String>> files = (Map<String,List<String>> ) in.readObject();
                                System.out.println("FileNo  --  File Name");
                                System.out.println("Public File:");
                                List<String> publicFiles = files.get("public");
                                int ind = 0;
                                for (String str: publicFiles){
                                    System.out.println(ind+"  --  "+str);
                                    ind++;
                                }
                                System.out.println("Private File:");
                                List<String> privateFiles = files.get("private");
                                for (String str: privateFiles){
                                    System.out.println(ind+"  --  "+str);
                                    ind++;
                                }
                                if(!fileTransfer){
                                    fileTransfer = true;
                                    System.out.println("Enter file no to download or -1 to back");
                                    int inp = Integer.valueOf(scanner.nextLine());
                                    if(inp>=0 && inp<ind){
                                        fileOut.writeUTF("DOWNLOAD");
                                        fileOut.writeUTF("OWN");
                                        File file;
                                        if(inp>=publicFiles.size()){
                                            inp-=publicFiles.size();
                                            fileOut.writeUTF("private");
                                            fileOut.writeUTF(privateFiles.get(inp));
                                            file = new File("clientStorage/"+username+"/"+privateFiles.get(inp));
                                        }
                                        else {
                                            fileOut.writeUTF("public");
                                            fileOut.writeUTF(publicFiles.get(inp));
                                            file = new File("clientStorage/"+username+"/"+publicFiles.get(inp));
                                        }
                                        fileOutputStream = new FileOutputStream(file);
                                        fileOut.writeUTF("ready to download");
                                    }
                                    else{
                                        fileTransfer = false;
                                        if(inp!=-1)
                                            System.out.println("Invalid input given!");
                                    }
                                }
                                else
                                    System.out.println("Download module busy. Try again later if you want to download");

                            }
                            else if(option.equalsIgnoreCase("3")){
                                System.out.println("FileNo  --  File Name");
                                out.writeObject("OTHERS-FILE-LIST");
                                Map<String,List<String>> files = (Map<String,List<String>> ) in.readObject();
                                int ind = 0;
                                for (Map.Entry<String,List<String>> fileEntry: files.entrySet()){
                                    System.out.println("File of "+fileEntry.getKey()+":");
                                    for (String file: fileEntry.getValue()){
                                        System.out.println(ind+"  --  "+file);
                                        ind++;
                                    }
                                }
                                if(!fileTransfer){
                                    fileTransfer = true;
                                    System.out.println("Enter file no to download or -1 to back");
                                    int inp = Integer.valueOf(scanner.nextLine());
                                    if(inp>=0 && inp<ind){
                                        fileOut.writeUTF("DOWNLOAD");
                                        fileOut.writeUTF("OTHERS");
                                        int occ=0;
                                        boolean isBreak = false;
                                        for (Map.Entry<String,List<String>> fileEntry: files.entrySet()){
                                            for (String file: fileEntry.getValue()){
                                                if(occ==inp){
                                                    fileOut.writeUTF(fileEntry.getKey());
                                                    fileOut.writeUTF(file);
                                                    File f = new File("clientStorage/"+username+"/"+file);
                                                    fileOutputStream = new FileOutputStream(f);
                                                    fileOut.writeUTF("ready to download");
                                                    isBreak = true;
                                                    break;
                                                }
                                                occ++;
                                            }
                                            if(isBreak)
                                                break;
                                        }
                                    }
                                    else{
                                        fileTransfer = false;
                                        if(inp!=-1)
                                            System.out.println("Invalid input given!");
                                    }
                                }
                                else
                                    System.out.println("Download module busy. Try again later if you want to download");
                            }
                            else if(option.equalsIgnoreCase("4")){
                                out.writeObject("MESSAGES");
                                List<String> messages = (List<String>) in.readObject();
                                System.out.println("Your unread messages");
                                for(String msg: messages){
                                    System.out.println(msg);
                                }
                            }
                            else if(option.equalsIgnoreCase("5")) {
                                if(fileTransfer){
                                    System.out.println("File Transfer module currently busy. Try again later.");
                                    continue;
                                }
                                fileTransfer = true;
                                fileOut.writeUTF("UPLOAD");
                                File[] files = directory.listFiles();
                                if (files != null) {
                                    System.out.println("FileNo -- File name --  size ");
                                    int c=0;
                                    for (File file : files) {
                                        if (file.isFile()) {
                                            System.out.println(c+" -- "+file.getName()+" -- "+file.length());
                                            c++;
                                        }
                                    }
                                    System.out.println("Enter File No which you want to upload. -1 to skip");
                                    int ind = Integer.valueOf(scanner.nextLine());
                                    if(!(ind>=0 && ind<c)){
                                        if(ind==-1)
                                            System.out.println("Terminated upload Request");
                                        else
                                            System.out.println("Invalid input given. Request terminated");
                                        fileOut.writeUTF("terminated");
                                        fileTransfer = false;
                                        continue;
                                    }

                                    File file = files[ind];
                                    System.out.println("File Name: "+file.getName()+" file size: "+file.length());
                                    fileOut.writeUTF(file.getName());
                                    fileOut.writeLong(file.length());
                                    System.out.println("Press 1 for upload as a public, 2 for as a private, 3 for as a requested file");
                                    String inp = scanner.nextLine();
                                    if(inp.equalsIgnoreCase("2")){
                                        fileOut.writeUTF("private");

                                    }
                                    else if(inp.equalsIgnoreCase("3")){
                                        fileOut.writeUTF("request");
                                        System.out.println("Enter Request ID:");
                                        String str = scanner.nextLine();
                                        fileOut.writeUTF(str);
                                    }
                                    else{
                                        fileOut.writeUTF("public");
                                    }

                                    fileInputStream = new FileInputStream(file);
                                    fileOut.writeUTF("Ready to upload");
                                }
                            }

                            else if(option.equalsIgnoreCase("6")){
                                out.writeObject("REQUEST");
                                System.out.println("Enter file description: ");
                                String desc = scanner.nextLine();
                                out.writeObject(desc);
                                System.out.println((String) in.readObject());
                            }
                            else if(option.equalsIgnoreCase("7")){
                                System.out.println("Logout");
                                out.writeObject("LOGOUT");
                                fileTransferThread.interrupt();
                                broadcastThread.interrupt();

                                socket1.close();
                                socket2.close();
                                socket3.close();
                                break;
                            }
                            else{
                                System.out.println("Invalid input given");
                            }
                        }

                    }

                }catch (Exception e){
                    //e.printStackTrace();
                    try {
                        out.writeObject("LOGOUT");
                    }catch (Exception e1){

                    }
                    System.out.println("Disconnected");
                }
            }
        });
        fileTransferThread = new Thread(new Runnable() {

            @Override
            public void run() {
                try {

                    while (true) {
                        String command = fileIn.readUTF();

                        switch (command) {
                            case "UPLOAD":
                                socket2.setSoTimeout(30000);
                                String fid = fileIn.readUTF();
                                if(fid.startsWith("Failed")){
                                    System.out.println(fid);
                                }
                                else {
                                    int chunkSize = fileIn.readInt();
                                    byte[] buffer = new byte[chunkSize];
                                    int bytesRead;
                                    System.out.println("Chunk size: "+chunkSize);
                                    while ((bytesRead = fileInputStream.read(buffer)) != -1) {

                                        fileOut.write(buffer, 0, bytesRead);
                                        System.out.println("Sent "+bytesRead+" byte");
                                        fileOut.flush();
                                        try {
                                            String ack = fileIn.readUTF();
                                            System.out.println(ack);
                                        }catch (SocketTimeoutException e){
                                            System.out.println("TIMEOUT");
                                            fileOut.writeUTF("  Timeout");
                                            fileOut.flush();
                                            fileInputStream.close();
                                            return;
                                        }
                                    }
                                    fileOut.writeUTF("Completed");
                                    System.out.println(fileIn.readUTF());
                                }

                                fileInputStream.close();
                                socket2.setSoTimeout(0);
                                fileTransfer=false;
                                print();
                                break;
                            case "DOWNLOAD":
                                int chunkSize = fileIn.readInt();
                                byte[] buffer = new byte[chunkSize];
                                int bytesRead;
                                while ((bytesRead = fileIn.read(buffer)) != -1) {
                                    String data = new String(buffer, bytesRead-9,9);
                                    if (data.equalsIgnoreCase("File Sent")){
                                        System.out.println(data);
                                        break;
                                    }
                                    fileOutputStream.write(buffer, 0, bytesRead);
                                    System.out.println("Received "+bytesRead+" bytes");
                                }
                                fileOutputStream.close();
                                fileTransfer=false;
                                print();
                                break;
                            default:
                                fileOut.writeUTF("ERROR: Invalid command.");
                                print();
                                break;
                        }
                    }
                }catch (Exception e){
                    try {
                        out.writeObject("LOGOUT");
                    }catch (Exception e1){

                    }
                    System.out.println("Disconnected");
                }
            }
        });

        broadcastThread = new Thread(new Runnable() {

            @Override
            public void run() {
                try {
                    while (true){
                        String msg = (String) broadcastIn.readObject();
                        System.out.println("New Broadcast Message:");
                        System.out.println(msg);
                    }
                }
                catch (Exception e){
                    try {
                        out.writeObject("LOGOUT");
                    }catch (Exception e1){

                    }
                    System.out.println("Disconnected");
                }
            }
        });
    commandThread.start();
    fileTransferThread.start();
    broadcastThread.start();
    }

    public static void main(String[] args) {
        scanner = new Scanner(System.in);
        Client client = new Client();

    }
}
