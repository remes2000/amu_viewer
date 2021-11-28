package pl.nieruchalski.client.domain.values.event;

import pl.nieruchalski.client.domain.exception.CannotCloseConnectionWithHostException;

import java.io.IOException;
import java.net.Socket;

public class ViewerHost implements AutoCloseable {
    private static Integer nextIdNumber = 1;
    private static Integer generateId() {
        Integer id = nextIdNumber;
        nextIdNumber++;
        return id;
    }

    private Integer id;
    private Socket socket;
    private Integer udpPort;
    private Frame lastFrame;

    public ViewerHost(Socket socket, Integer udpPort) {
        this.id = generateId();
        this.socket = socket;
        this.udpPort = udpPort;
    }

    @Override
    public void close() throws CannotCloseConnectionWithHostException {
        try {
            socket.close();
        } catch (IOException e) {
            throw new CannotCloseConnectionWithHostException();
        }
    }

    public String getName() {
        return this.getIp();
    }

    public String getIp() {
        return this.socket.getInetAddress().getHostAddress();
    }

    public Integer getCommunicationTcpPort() {
        return this.socket.getPort();
    }

    public Integer getFileTransferTcpPort() {
        return null;
    }

    public Integer getUdpPort() {
        return this.udpPort;
    }

    public Frame getLastFrame() {
        return lastFrame;
    }

    public void setLastFrame(Frame lastFrame) {
        this.lastFrame = lastFrame;
    }
}
