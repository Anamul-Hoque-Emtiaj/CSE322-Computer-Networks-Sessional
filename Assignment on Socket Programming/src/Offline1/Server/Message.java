package Offline1.Server;

public class Message {
    protected boolean isRead;
    protected FileRequest file;
    protected String client; //who uploaded file
    protected String broadcastMsg;

    public Message(FileRequest file, String client) {
        this.file = file;
        this.client = client;
        isRead = false;
        broadcastMsg = null;
    }

    public Message(String broadcastMsg) {
        this.broadcastMsg = broadcastMsg;
        isRead = false;
        file = null;
        client = null;
    }
}
