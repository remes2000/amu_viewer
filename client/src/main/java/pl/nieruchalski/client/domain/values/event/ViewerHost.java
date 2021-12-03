package pl.nieruchalski.client.domain.values.event;

import javafx.scene.input.KeyCode;
import pl.nieruchalski.client.domain.exception.CannotCloseConnectionWithHostException;
import pl.nieruchalski.client.domain.helpers.EventCodes;
import pl.nieruchalski.client.domain.helpers.KeyCodeMap;
import pl.nieruchalski.client.domain.helpers.ViewerTcpSocket;

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
    private ViewerTcpSocket socket;
    private Integer udpPort;
    private Frame lastFrame;

    public ViewerHost(ViewerTcpSocket socket, Integer udpPort) {
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
        return this.socket.getSocket().getInetAddress().getHostAddress();
    }

    public Integer getCommunicationTcpPort() {
        return this.socket.getSocket().getPort();
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

    public Integer getId() {
        return this.id;
    }

    public void mouseMove(Integer x, Integer y) {
        this.mouseEvent(x, y, EventCodes.MOUSE_MOVE);
    }

    public void mouseLeftPressed(Integer x, Integer y) {
        this.mouseEvent(x, y, EventCodes.MOUSE_LEFT_PRESSED);
    }

    public void mouseLeftReleased(Integer x, Integer y) {
        this.mouseEvent(x, y, EventCodes.MOUSE_LEFT_RELEASED);
    }

    public void mouseRightPressed(Integer x, Integer y) {
        this.mouseEvent(x, y, EventCodes.MOUSE_RIGHT_PRESSED);
    }

    public void mouseRightReleased(Integer x, Integer y) {
        this.mouseEvent(x, y, EventCodes.MOUSE_RIGHT_RELEASED);
    }

    public void keyPressed(KeyCode keyCode) {
        Integer translatedKeyCode = KeyCodeMap.translate(keyCode);
        if(translatedKeyCode == null) {
            return;
        }
        try {
            this.socket.getOutputStream().writeShort(EventCodes.KEY_PRESSED);
            this.socket.getOutputStream().writeInt(translatedKeyCode);
        } catch (IOException e) {
            System.err.println("Cannot execute key pressed event");
        }
    }

    public void keyReleased(KeyCode keyCode) {
        Integer translatedKeyCode = KeyCodeMap.translate(keyCode);
        if(translatedKeyCode == null) {
            return;
        }
        try {
            this.socket.getOutputStream().writeShort(EventCodes.KEY_RELEASED);
            this.socket.getOutputStream().writeInt(translatedKeyCode);
        } catch (IOException e) {
            System.err.println("Cannot execute key released event");
        }
    }

    private void mouseEvent(Integer x, Integer y, short code) {
        try {
            this.socket.getOutputStream().writeShort(code);
            this.socket.getOutputStream().writeInt(x);
            this.socket.getOutputStream().writeInt(y);
        } catch (IOException e) {
            System.err.println("Cannot execute mouse event");
        }
    }
}
