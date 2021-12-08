package pl.nieruchalski.client.domain.helpers;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.Socket;

public class ViewerTcpSocket implements AutoCloseable {
    private Socket socket;
    private DataInputStream inputStream;
    private DataOutputStream outputStream;

    public ViewerTcpSocket(Socket socket) throws IOException {
        this.socket = socket;
        this.inputStream = new DataInputStream(socket.getInputStream());
        this.outputStream = new DataOutputStream(socket.getOutputStream());
    }

    public Socket getSocket() {
        return this.socket;
    }

    public DataInputStream getInputStream() {
        return inputStream;
    }

    public DataOutputStream getOutputStream() {
        return outputStream;
    }

    public void close() throws IOException {
        this.socket.close();
    }
}
