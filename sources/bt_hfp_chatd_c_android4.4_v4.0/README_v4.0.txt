bt_hfp_chatd_c v4.0 continuation package

Scope of this drop:
- Continues from the stated v3.9 baseline architecture.
- Adds stronger MAP MAS SDP advertisement foundation.
- Adds MAP notification service (MNS) foundation and event queue.
- Extends message storage so MSG_SEND / MSG_LIST / MSG_CLEAR can track handles, status, read-state, and change ids.
- Keeps pure C, NDK-friendly, no CMake.

What is foundation-only in this package:
- MAS SDP record generation is implemented, but actual registration with the platform Bluetooth stack still must be connected in your daemon / JNI / vendor stack integration layer.
- MNS notification queue and event XML generation are implemented, but the real OBEX transport/session handling is not included here.
- Interoperability behavior for specific smart headsets still needs real device testing.

What is closer to production-ready:
- Internal data model for message records.
- Notification registration state handling.
- Event report XML construction.
- Clear split between store / MAS / MNS / SDP generation.

Recommended integration order:
1. Replace or merge msg_store.c/.h with your v3.9 MSG foundation.
2. Replace or merge map_mse.c/.h.
3. Add map_mns.c/.h and map_sdp.c/.h.
4. Wire map_mse_get_sdp_xml() into your existing SDP registration path.
5. Wire map_mns_build_event_report_xml() into your existing RFCOMM/OBEX path for remote notification transport.
6. Test with at least one B7-like headset and one second vendor headset.

Main interoperability intent for v4.0:
- Better MAS service advertisement fields.
- Notification registration support.
- Event generation for new message / sending success / delivery success / sending failure.

Known gaps intentionally not hidden:
- No full MAP 1.4 parser/serializer.
- No folder browsing protocol implementation.
- No real MNS server socket / OBEX connect sequence in this drop.
- No headset-specific quirk table yet.
