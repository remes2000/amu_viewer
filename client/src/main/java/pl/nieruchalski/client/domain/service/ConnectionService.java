package pl.nieruchalski.client.domain.service;

import javafx.application.Platform;
import pl.nieruchalski.client.domain.exception.CannotOpenUdpSocketException;
import pl.nieruchalski.client.domain.exception.EstablishConnectionException;
import pl.nieruchalski.client.domain.exception.HostRefusedAccessCodeException;
import pl.nieruchalski.client.domain.publisher.GeneralPublisher;
import pl.nieruchalski.client.domain.publisher.NewConnectionPublisher;
import pl.nieruchalski.client.domain.values.ConnectionRequest;
import pl.nieruchalski.client.domain.values.event.ViewerHost;
import pl.nieruchalski.client.domain.values.event.general.UdpPortChange;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.DatagramSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.HashSet;
import java.util.Set;

public class ConnectionService {
    private static ConnectionService service;
    private DatagramSocket udpSocket;
    private Set<ViewerHost> activeHosts = new HashSet();

    public static ConnectionService getInstance() {
        if(service == null) {
            service = new ConnectionService();
        }
        return service;
    }

    private ConnectionService() {
        this.initUdpSocket();
    }

    private void initUdpSocket() {
        try {
            this.udpSocket = new DatagramSocket();
            GeneralPublisher.getInstance().broadcast(new UdpPortChange(this.udpSocket.getLocalPort()));
            new FrameReceiver(udpSocket).start();
        } catch (SocketException e) {
            throw new CannotOpenUdpSocketException(e);
        }
    }

    public void establishConnection(ConnectionRequest connectionRequest) throws EstablishConnectionException, HostRefusedAccessCodeException {
        try {
            Socket socket = new Socket(connectionRequest.getIp(), connectionRequest.getPort());
            DataOutputStream outputStream = new DataOutputStream(socket.getOutputStream());
            DataInputStream inputStream = new DataInputStream(socket.getInputStream());
            outputStream.writeInt(connectionRequest.getAccessCode());
            if(inputStream.readByte() == 0) {
                throw new HostRefusedAccessCodeException();
            }
            outputStream.writeInt(this.udpSocket.getLocalPort());
            ViewerHost host = new ViewerHost(socket);
            NewConnectionPublisher.getInstance().broadcast(host);
        } catch (IOException e) {
            throw new EstablishConnectionException();
        }
    }
}
