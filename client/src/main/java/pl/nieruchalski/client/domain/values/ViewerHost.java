package pl.nieruchalski.client.domain.values;

import java.net.Socket;

public class ViewerHost {
    private String ipAddress;
    private Integer port;
    private Socket socket;

    public ViewerHost(String ipAddress, Integer port, Socket socket) {
        this.ipAddress = ipAddress;
        this.port = port;
        this.socket = socket;
    }
}
