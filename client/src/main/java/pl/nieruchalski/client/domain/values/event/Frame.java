package pl.nieruchalski.client.domain.values.event;

import java.net.DatagramPacket;
import java.util.Arrays;

public class Frame {
    public byte[] buffer;
    public String ipAddress;
    public Integer port;

    public Frame(byte[] buffer, String ipAddress, Integer port) {
        this.buffer = buffer;
        this.ipAddress = ipAddress;
        this.port = port;
    }

    public Frame(DatagramPacket packet) {
        this(Arrays.copyOf(packet.getData(), packet.getLength()), packet.getAddress().getCanonicalHostName(), packet.getPort());
    }
}
