package pl.nieruchalski.client.domain.service;

import pl.nieruchalski.client.domain.publisher.GeneralPublisher;
import pl.nieruchalski.client.domain.values.event.ViewerHost;

import java.net.DatagramPacket;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class HostManager {
    private static HostManager hostManager;

    private HostManager() {}

    public static HostManager getInstance() {
        if(hostManager == null) {
            hostManager = new HostManager();
        }
        return hostManager;
    }

    private Set<ViewerHost> activeHosts = new HashSet();

    private ViewerHost selectedHost;

    public void selectHost(ViewerHost host) {
        this.selectedHost = host;
    }

    public ViewerHost getSelectedHost() {
        return this.selectedHost;
    }

    public void registerHost(ViewerHost host) {
        activeHosts.add(host);
    }

    public void unregisterHost(ViewerHost host) {
        activeHosts.remove(host);
    }

    public ViewerHost getPacketSender(DatagramPacket packet) {
        String packetIp = packet.getAddress().getHostAddress();
        Integer packetPort = packet.getPort();

        for(ViewerHost host : activeHosts) {
            if(host.getIp().equals(packetIp) && host.getUdpPort().equals(packetPort)) {
                return host;
            }
        }
        return null;
    }
}
