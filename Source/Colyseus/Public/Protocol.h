#pragma once

enum class Protocol : int
{
	// Room-related (10~19)
	HANDSHAKE = 9,
	JOIN_ROOM = 10,
	JOIN_ERROR = 11,
	LEAVE_ROOM = 12,
	ROOM_DATA = 13,
	ROOM_STATE = 14,
	ROOM_STATE_PATCH = 15,
	ROOM_DATA_SCHEMA = 16,
};
