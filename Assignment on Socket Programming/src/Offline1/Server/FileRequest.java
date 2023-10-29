package Offline1.Server;

public class FileRequest {
    protected String description;
    protected String reqID;
    protected String client;

    public FileRequest(String description, String reqID, String client) {
        this.description = description;
        this.reqID = reqID;
        this.client = client;
    }
}
