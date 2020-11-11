
#ifndef __SAL_VOIP_H__
#define __SAL_VOIP_H__


//.VoiceService.{i}.VoiceProfile.{i}.Line.{i}.Codec.
char *sal_voip_get_tx_codec_t(int line);
char *sal_voip_get_tx_vad_t(int line);
char *sal_voip_get_tx_ptime_t(int line);

//.VoiceService.{i}.VoiceProfile.{i}.Line.{i}.Session.{i}.
char *sal_voip_get_session_startime_t(int line);
char *sal_voip_get_session_cutime_t(int line);
char *sal_voip_get_far_end_ip_t(int line);
char *sal_voip_get_local_UDP_port_t(int line);
char *sal_voip_get_far_UDP_port_t(int line);

//.VoiceService.{i}.VoiceProfile.{i}.Line.{i}.Stats.
char *sal_voip_get_packets_sent_t(int line);
char *sal_voip_get_packets_receive_t(int line);
char *sal_voip_get_bytes_sent_t(int line);
char *sal_voip_get_bytes_receive_t(int line);
char *sal_voip_get_packets_lost_t(int line);
char *sal_voip_get_overruns_t(int line);
char *sal_voip_get_underruns_t(int line);
//ReceivePacketLossRate
char *sal_voip_get_rplr_t(int line);
//FarEndPacketLossRate
char *sal_voip_get_feplr_t(int line);
//ReceiveInterarrivalJitter
char *sal_voip_get_rij_t(int line);
//FarEndInterarrivalJitter
char *sal_voip_get_feij_t(int line);
//RoundTripDelay
char *sal_voip_get_rtd_t(int line);
//AverageReceiveInterarrivalJitter
char *sal_voip_get_arij_t(int line);
//AverageFarEndInterarrivalJitter
char *sal_voip_get_afeij_t(int line);
//AverageRoundTripDelay
char *sal_voip_get_artd_t(int line);

char *sal_voip_get_incall_received_t(int line);
char *sal_voip_get_incall_answered_t(int line);
char *sal_voip_get_incall_connected_t(int line);
char *sal_voip_get_incall_failed_t(int line);
char *sal_voip_get_outcall_attempted_t(int line);
char *sal_voip_get_outcall_answered_t(int line);
char *sal_voip_get_outcall_connected_t(int line);
char *sal_voip_get_outcall_failed_t(int line);
char *sal_voip_get_callsdropped_t(int line);
char *sal_voip_get_total_time_t(int line);

 //for GPON
char *sal_voip_get_hook_state_t(int line);
char *sal_voip_get_session_state_t(int line);

//for call state
char *sal_voip_get_call_state_t(int line);

/*for lan call state*/
char *sal_voip_get_lan_call_state_t(int line);

//for register state
char *sal_voip_get_reg_state_t(int line);

//for server down time
char *sal_voip_get_server_down_time_t(int line);

#endif
