package Offline1.Server;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.*;

public class Server {
    private static final int SERVER_PORT1 = 6666;
    private static final int SERVER_PORT2 = 6667;
    private static final int SERVER_PORT3 = 6668;
    protected static final  int MAX_BUFFER_SIZE = 100000;
    protected static final int MIN_CHUNK_SIZE = 500;
    protected static final int MAX_CHUNK_SIZE = 10000;

    private ServerSocket serverSocket1;
    private ServerSocket serverSocket2;
    private ServerSocket serverSocket3;

    protected Map<String, ClientHandler> clientHandlers;
    protected Map<String, List<Message>> clientMessages;
    protected Map<String,FileRequest> requestedFiles;

    protected int reqId=1;
    protected int fileID = 1;
    protected byte[] buffer;
    protected long usedBufferSize;


    public Server() {

        clientMessages = new HashMap<>();
        requestedFiles = new HashMap<>();
        clientHandlers = new HashMap<>();
        buffer = new byte[MAX_BUFFER_SIZE];
        usedBufferSize = 0;
    }

    public void start(){
        try {
            serverSocket1 = new ServerSocket(SERVER_PORT1);
            serverSocket2 = new ServerSocket(SERVER_PORT2);
            serverSocket3 = new ServerSocket(SERVER_PORT3);
            System.out.println("Server started");

            while (true) {
                Socket clientSocket1 = serverSocket1.accept();
                Socket clientSocket2 = serverSocket2.accept();
                Socket clientSocket3 = serverSocket3.accept();
                ClientHandler clientThread = new ClientHandler(clientSocket1,clientSocket2,clientSocket3,this);
            }
        } catch (IOException e) {
            System.out.println("Server Failed");
        }
    }

    protected synchronized List<String> getMessage(String client){
        List<String> msg = new ArrayList<>();
        List<Message> messages = clientMessages.get(client);
        for (Message message: messages){
            String m;
            if (message.broadcastMsg==null){
                m = message.client+" uploaded your requested file.\nRequest Id: "+message.file.reqID+"\nFile Description: "+message.file.description;
            }else {
                m = message.broadcastMsg;
            }
            if(!message.isRead){
                msg.add("(unread) "+m);
                message.isRead = true;
            }
            else
                msg.add("(read) "+m);
        }
        if(msg.isEmpty())
            msg.add("No Message");
        return msg;
    }
    protected synchronized int genRandomChunkSize(){
        Random random = new Random();
        return random.nextInt(MAX_CHUNK_SIZE - MIN_CHUNK_SIZE + 1) + MIN_CHUNK_SIZE;
    }


    protected synchronized String addNewFileRequest(String client, String desc) throws Exception{
        String rid = "r-"+Integer.toString(reqId);
        reqId++;
        FileRequest file = new FileRequest(desc,rid,client);
        requestedFiles.put(rid,file);
        for(Map.Entry<String,ClientHandler> clientHandlerEntry: clientHandlers.entrySet()){
            ClientHandler cur = clientHandlerEntry.getValue();
            if(!clientHandlerEntry.getKey().equalsIgnoreCase(client)){
                if(cur.isLogIn)
                    cur.broadcast("New file requested from "+client+"\nRequest id: "+rid+"\nFile description: "+desc);
                else{
                    List<Message> messages = clientMessages.get(cur.username);
                    messages.add(new Message("New file requested from "+client+" while you are in offline.\nRequest id: "+rid+"\nFile description: "+desc));
                    clientMessages.put(cur.username,messages);
                }
            }
        }
        return rid;
    }

    public static void main(String[] args) {
        Server fileServer = new Server();
        fileServer.start();
    }
}
