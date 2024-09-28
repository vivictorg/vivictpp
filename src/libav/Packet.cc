// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "libav/Packet.hh"

void freePacket(AVPacket *pkt) {
  if (pkt) {
    av_packet_free(&pkt);
  }
}

vivictpp::libav::Packet::Packet() : packet() {}

vivictpp::libav::Packet::Packet(AVPacket *pkt)
    : packet(pkt ? av_packet_clone(pkt) : pkt, &freePacket) {}

vivictpp::libav::Packet::Packet(bool _eof) : _eof(_eof) {}

AVPacket *vivictpp::libav::Packet::avPacket() { return packet.get(); }
