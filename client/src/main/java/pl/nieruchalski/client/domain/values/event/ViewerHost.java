package pl.nieruchalski.client.domain.values.event;

import javafx.scene.input.KeyCode;
import pl.nieruchalski.client.domain.exception.CannotCloseConnectionWithHostException;
import pl.nieruchalski.client.domain.exception.CannotSendFileException;
import pl.nieruchalski.client.domain.exception.HostRefusedAccessCodeException;
import pl.nieruchalski.client.domain.helpers.EventCodes;
import pl.nieruchalski.client.domain.helpers.KeyCodeMap;
import pl.nieruchalski.client.domain.helpers.ViewerTcpSocket;
import pl.nieruchalski.client.domain.service.FileSender;

import java.io.File;
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
    private Integer fileTransferTcpPort;
    private Integer accessCode;
    private Frame lastFrame;

    public ViewerHost(ViewerTcpSocket socket, Integer udpPort, Integer fileTransferTcpPort, Integer accessCode) {
        this.id = generateId();
        this.socket = socket;
        this.udpPort = udpPort;
        this.fileTransferTcpPort = fileTransferTcpPort;
        this.accessCode = accessCode;
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
        return this.fileTransferTcpPort;
    }

    public Integer getUdpPort() {
        return this.udpPort;
    }

    public Frame getLastFrame() {
        return lastFrame;
    }

    public Integer getAccessCode() {
        return accessCode;
    }

    public void setLastFrame(Frame lastFrame) {
        this.lastFrame = lastFrame;
    }

    public Integer getId() {
        return this.id;
    }

    public void sendFile(File file) throws CannotSendFileException, HostRefusedAccessCodeException {
        new FileSender(this, file).send();
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
        this.keyEvent(keyCode, EventCodes.KEY_PRESSED);
    }

    public void keyReleased(KeyCode keyCode) {
        this.keyEvent(keyCode, EventCodes.KEY_RELEASED);
    }

    private void keyEvent(KeyCode keyCode, short eventCode) {
        Integer translatedKeyCode = KeyCodeMap.translate(keyCode);
        if(translatedKeyCode == null) {
            return;
        }
        try {
            this.socket.getOutputStream().writeShort(eventCode);
            this.socket.getOutputStream().writeInt(translatedKeyCode);
        } catch (IOException e) {
            System.err.println("Cannot execute key event");
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
