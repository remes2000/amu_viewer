package pl.nieruchalski.client.domain.service;

import javafx.application.Platform;
import pl.nieruchalski.client.domain.publisher.NewFramePublisher;
import pl.nieruchalski.client.domain.values.event.Frame;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;

public class FrameReceiver extends Thread {
    private static final int MAX_FRAME_SIZE = 65535;
    private byte[] buffer = new byte[MAX_FRAME_SIZE];

    private DatagramSocket socket;

    public FrameReceiver(DatagramSocket socket) {
        this.socket = socket;
    }

    @Override
    public void run() {
        DatagramPacket packet = new DatagramPacket(this.buffer, MAX_FRAME_SIZE);
        while(true) {
            try {
                this.socket.receive(packet);
                Platform.runLater(() -> {
                    NewFramePublisher.getInstance().broadcast(new Frame(packet));
                });
            } catch (IOException e) {
                System.err.println("An error occured while receiving frame via udp");
            }
        }
    }
}
