package pl.nieruchalski.client.domain.service;

import pl.nieruchalski.client.domain.exception.CannotSendFileException;
import pl.nieruchalski.client.domain.exception.HostRefusedAccessCodeException;
import pl.nieruchalski.client.domain.helpers.ViewerTcpSocket;
import pl.nieruchalski.client.domain.values.event.ViewerHost;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.Socket;

public class FileSender {
    private ViewerHost host;
    private File file;

    public FileSender(ViewerHost host, File file) {
        this.host = host;
        this.file = file;
    }

    public void send() throws CannotSendFileException, HostRefusedAccessCodeException {
        try (
            ViewerTcpSocket socket = new ViewerTcpSocket(new Socket(this.host.getIp(), this.host.getFileTransferTcpPort()));
            FileInputStream fileInputStream = new FileInputStream(this.file);
        ){
            socket.getOutputStream().writeInt(this.host.getAccessCode());
            if(socket.getInputStream().readByte() == 0) {
                socket.close();
                throw new HostRefusedAccessCodeException();
            }
            socket.getOutputStream().writeInt(this.file.getName().length());
            socket.getOutputStream().writeBytes(this.file.getName());
            socket.getOutputStream().writeInt((int) this.file.length()); // I know I shouldn't do it, don't have time to implement 64-byte ntoh method client side
            int read_bytes = 0;
            while((read_bytes = fileInputStream.read()) != -1) {
                socket.getOutputStream().writeByte(read_bytes);
            }
        } catch (IOException exception) {
            exception.printStackTrace();
            throw new CannotSendFileException();
        }
    }
}
