package pl.nieruchalski.client.domain.values.event;

import java.net.Socket;

public class ViewerHost {
    private Socket socket;

    public ViewerHost(Socket socket) {
        this.socket = socket;
    }

    public String getName() {
        return this.socket.getInetAddress().getCanonicalHostName();
    }
}
