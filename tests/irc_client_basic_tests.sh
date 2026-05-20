#!/bin/bash

# AUTHENTICATION TESTS (PASS, NICK, USER)

# Test 1: Valid authentication sequence
auth_valid_sequence=( \
	"PASS password" \
	"NICK testuser1" \
	"USER testuser1 0 0 :Test User One" \
	"" \
)

# Test 2: Authentication with spaces in real name
auth_with_realname_spaces=( \
	"PASS password" \
	"NICK user2" \
	"USER user2 0 0 :John Doe Smith" \
	"" \
)

# Test 3: Wrong password (should fail)
auth_wrong_password=( \
	"PASS wrongpassword" \
	"NICK baduser" \
	"USER baduser 0 0 :Bad User" \
	"" \
)

# Test 4: No password (should fail if required)
auth_no_password=( \
	"NICK nopassuser" \
	"USER nopassuser 0 0 :No Pass User" \
	"" \
)

# Test 5: NICK before PASS
auth_nick_before_pass=( \
	"NICK premature" \
	"PASS password" \
	"USER premature 0 0 :Premature User" \
	"" \
)

# Test 6: NICK command only (incomplete registration)
auth_nick_only=( \
	"PASS password" \
	"NICK nickonly" \
	"" \
)

# Test 7: USER command only (incomplete registration)
auth_user_only=( \
	"PASS password" \
	"USER useronly 0 0 :User Only" \
	"" \
)

# Test 8: Duplicate NICK in sequence
auth_duplicate_nick=( \
	"PASS password" \
	"NICK dupnick" \
	"NICK dupnick" \
	"USER dupnick 0 0 :Dup Nick" \
	"" \
)

# Test 9: NICK change mid-registration
auth_nick_change=( \
	"PASS password" \
	"NICK oldnick" \
	"NICK newnick" \
	"USER newnick 0 0 :New Nick User" \
	"" \
)

# Test 10: Empty/whitespace NICK
auth_empty_nick=( \
	"PASS password" \
	"NICK " \
	"" \
)

# Test 11: Special characters in nick
auth_special_chars_nick=( \
	"PASS password" \
	"NICK nick@#$%" \
	"USER nick@#$% 0 0 :Special Nick" \
	"" \
)

# Test 12: Very long nick
auth_long_nick=( \
	"PASS password" \
	"NICK verylongnicknamethatmightexceedlimitbutletustest" \
	"USER verylongnicknamethatmightexceedlimitbutletustest 0 0 :Long Nick" \
	"" \
)

# Test 13: Empty real name
auth_empty_realname=( \
	"PASS password" \
	"NICK emptyreal" \
	"USER emptyreal 0 0 :" \
	"" \
)

# Test 14: Very long real name
auth_long_realname=( \
	"PASS password" \
	"NICK longreal" \
	"USER longreal 0 0 :This is a very long real name that might exceed system limits and we want to see how the server handles it" \
	"" \
)

# Test 15: Multiple spaces in USER command
auth_multiple_spaces_user=( \
	"PASS password" \
	"NICK spaceuser" \
	"USER  spaceuser  0  0  :Space User" \
	"" \
)

# CHANNEL TESTS (JOIN, PART, PRIVMSG)

# Test 16: Simple channel join
channel_simple_join=( \
	"PASS password" \
	"NICK chan1user" \
	"USER chan1user 0 0 :Channel One User" \
	"JOIN #general" \
	"" \
)

# Test 17: Join multiple channels
channel_multiple_joins=( \
	"PASS password" \
	"NICK multiuser" \
	"USER multiuser 0 0 :Multi User" \
	"JOIN #general" \
	"JOIN #test" \
	"JOIN #random" \
	"" \
)

# Test 18: Join without # prefix
channel_join_no_hash=( \
	"PASS password" \
	"NICK nohash" \
	"USER nohash 0 0 :No Hash" \
	"JOIN general" \
	"" \
)

# Test 19: Join empty channel (should create it)
channel_join_creates_new=( \
	"PASS password" \
	"NICK creator" \
	"USER creator 0 0 :Creator" \
	"JOIN #brandnewchannel12345" \
	"" \
)

# Test 20: Join with channel key
channel_join_with_key=( \
	"PASS password" \
	"NICK keyuser" \
	"USER keyuser 0 0 :Key User" \
	"JOIN #secret secretpass" \
	"" \
)

# Test 21: Part from channel
channel_simple_part=( \
	"PASS password" \
	"NICK partuser" \
	"USER partuser 0 0 :Part User" \
	"JOIN #general" \
	"PART #general" \
	"" \
)

# Test 22: Part with message
channel_part_with_message=( \
	"PASS password" \
	"NICK partmsg" \
	"USER partmsg 0 0 :Part Message" \
	"JOIN #general" \
	"PART #general :Goodbye everyone!" \
	"" \
)

# Test 23: Part from channel not joined
channel_part_not_joined=( \
	"PASS password" \
	"NICK notjoined" \
	"USER notjoined 0 0 :Not Joined" \
	"PART #general" \
	"" \
)

# Test 24: Send message to channel
channel_privmsg_to_channel=( \
	"PASS password" \
	"NICK msguser" \
	"USER msguser 0 0 :Msg User" \
	"JOIN #general" \
	"PRIVMSG #general :Hello everyone!" \
	"" \
)

# Test 25: Send message without joining
channel_privmsg_not_joined=( \
	"PASS password" \
	"NICK notinch" \
	"USER notinch 0 0 :Not In Channel" \
	"PRIVMSG #general :This should fail" \
	"" \
)

# Test 26: Empty channel message
channel_empty_message=( \
	"PASS password" \
	"NICK emptymsg" \
	"USER emptymsg 0 0 :Empty Msg" \
	"JOIN #general" \
	"PRIVMSG #general :" \
	"" \
)

# Test 27: Very long channel message
channel_long_message=( \
	"PASS password" \
	"NICK longmsg" \
	"USER longmsg 0 0 :Long Msg" \
	"JOIN #general" \
	"PRIVMSG #general :This is a very long message that tests the server's buffer handling and might exceed normal line lengths or socket buffer sizes so we keep adding more text here to make it even longer" \
	"" \
)

# Test 28: Message with special characters
channel_special_chars_message=( \
	"PASS password" \
	"NICK specialmsg" \
	"USER specialmsg 0 0 :Special Msg" \
	"JOIN #general" \
	"PRIVMSG #general :Special chars: !@#$%^&*()[]{}|;:',.<>?/" \
	"" \
)

# Test 29: Message with newlines/escaped sequences
channel_escaped_message=( \
	"PASS password" \
	"NICK escapemsg" \
	"USER escapemsg 0 0 :Escape Msg" \
	"JOIN #general" \
	"PRIVMSG #general :Line1\\r\\nLine2" \
	"" \
)

# PRIVATE MESSAGE TESTS

# Test 30: Send private message to another user
privmsg_direct=( \
	"PASS password" \
	"NICK sender" \
	"USER sender 0 0 :Sender User" \
	"PRIVMSG receiver :Hello receiver!" \
	"" \
)

# Test 31: Send private message to non-existent user
privmsg_nonexistent=( \
	"PASS password" \
	"NICK sender2" \
	"USER sender2 0 0 :Sender Two" \
	"PRIVMSG nonexistentuser12345 :This should fail" \
	"" \
)

# Test 32: Empty private message
privmsg_empty=( \
	"PASS password" \
	"NICK sender3" \
	"USER sender3 0 0 :Sender Three" \
	"PRIVMSG someuser :" \
	"" \
)

# Test 33: Private message with special characters
privmsg_special=( \
	"PASS password" \
	"NICK specialsender" \
	"USER specialsender 0 0 :Special Sender" \
	"PRIVMSG targetuser :!@#$%^&*()[]{}|;:',.<>?/" \
	"" \
)

# CHANNEL MODE TESTS (MODE +i, +k, +l, +t, +o)

# Test 34: Set invite-only mode (+i)
mode_invite_only=( \
	"PASS password" \
	"NICK opuser" \
	"USER opuser 0 0 :Op User" \
	"JOIN #testmode" \
	"MODE #testmode +i" \
	"" \
)

# Test 35: Remove invite-only mode (-i)
mode_remove_invite=( \
	"PASS password" \
	"NICK opuser2" \
	"USER opuser2 0 0 :Op User Two" \
	"JOIN #testmode2" \
	"MODE #testmode2 +i" \
	"MODE #testmode2 -i" \
	"" \
)

# Test 36: Set channel key (+k)
mode_set_key=( \
	"PASS password" \
	"NICK keyop" \
	"USER keyop 0 0 :Key Op" \
	"JOIN #keytest" \
	"MODE #keytest +k secretpass" \
	"" \
)

# Test 37: Remove channel key (-k)
mode_remove_key=( \
	"PASS password" \
	"NICK keyop2" \
	"USER keyop2 0 0 :Key Op Two" \
	"JOIN #keytest2" \
	"MODE #keytest2 +k secretpass" \
	"MODE #keytest2 -k" \
	"" \
)

# Test 38: Set user limit (+l)
mode_set_limit=( \
	"PASS password" \
	"NICK limitop" \
	"USER limitop 0 0 :Limit Op" \
	"JOIN #limitest" \
	"MODE #limitest +l 5" \
	"" \
)

# Test 39: Remove user limit (-l)
mode_remove_limit=( \
	"PASS password" \
	"NICK limitop2" \
	"USER limitop2 0 0 :Limit Op Two" \
	"JOIN #limitest2" \
	"MODE #limitest2 +l 5" \
	"MODE #limitest2 -l" \
	"" \
)

# Test 40: Set topic restriction (+t)
mode_topic_restricted=( \
	"PASS password" \
	"NICK topicop" \
	"USER topicop 0 0 :Topic Op" \
	"JOIN #topictest" \
	"MODE #topictest +t" \
	"" \
)

# Test 41: Remove topic restriction (-t)
mode_remove_topic_restriction=( \
	"PASS password" \
	"NICK topicop2" \
	"USER topicop2 0 0 :Topic Op Two" \
	"JOIN #topictest2" \
	"MODE #topictest2 +t" \
	"MODE #topictest2 -t" \
	"" \
)

# Test 42: Grant operator privilege (+o)
mode_grant_op=( \
	"PASS password" \
	"NICK opgranter" \
	"USER opgranter 0 0 :Op Granter" \
	"JOIN #optest" \
	"MODE #optest +o anotheruser" \
	"" \
)

# Test 43: Remove operator privilege (-o)
mode_remove_op=( \
	"PASS password" \
	"NICK opremover" \
	"USER opremover 0 0 :Op Remover" \
	"JOIN #optest2" \
	"MODE #optest2 +o someuser" \
	"MODE #optest2 -o someuser" \
	"" \
)

# Test 44: Multiple mode changes at once
mode_multiple_changes=( \
	"PASS password" \
	"NICK multimode" \
	"USER multimode 0 0 :Multi Mode" \
	"JOIN #multitest" \
	"MODE #multitest +i +k secret +l 10" \
	"" \
)

# Test 45: Combine modes in single command
mode_combined=( \
	"PASS password" \
	"NICK combineop" \
	"USER combineop 0 0 :Combine Op" \
	"JOIN #combinetest" \
	"MODE #combinetest +ik secret" \
	"" \
)

# Test 46: Mode on non-existent channel
mode_nonexistent_channel=( \
	"PASS password" \
	"NICK modeuser" \
	"USER modeuser 0 0 :Mode User" \
	"MODE #doesntexist +i" \
	"" \
)

# Test 47: Non-op setting mode (should fail)
mode_non_op_attempt=( \
	"PASS password" \
	"NICK regularuser" \
	"USER regularuser 0 0 :Regular User" \
	"JOIN #protectedchannel" \
	"MODE #protectedchannel +i" \
	"" \
)

# Test 48: Invalid mode character
mode_invalid_character=( \
	"PASS password" \
	"NICK badmode" \
	"USER badmode 0 0 :Bad Mode" \
	"JOIN #badmodetest" \
	"MODE #badmodetest +x" \
	"" \
)

# OPERATOR TESTS (KICK, INVITE, TOPIC)

#TODO(Jyry): These do feel like they would work, because they are multiuser, setup needing tests. I will need to create a setup script for these tests.
## Test 49: Kick user from channel
#op_kick_user=( \
#	"PASS password" \
#	"NICK kicker" \
#	"USER kicker 0 0 :Kicker" \
#	"JOIN #kicktest" \
#	"KICK #kicktest targetuser" \
#	"" \
#)
#
## Test 50: Kick with reason
#op_kick_with_reason=( \
#	"PASS password" \
#	"NICK kicker2" \
#	"USER kicker2 0 0 :Kicker Two" \
#	"JOIN #kicktest2" \
#	"KICK #kicktest2 targetuser :You were not following the rules" \
#	"" \
#)
#
#op_kick_nonexistent=( \
#	"PASS password" \
#	"NICK kicker3" \
#	"USER kicker3 0 0 :Kicker Three" \
#	"JOIN #kicktest3" \
#	"KICK #kicktest3 nonexistentuser" \
#	"" \
#)
#
## Test 52: Non-op kick attempt (should fail)
#op_non_op_kick=( \
#	"PASS password" \
#	"NICK regularuser2" \
#	"USER regularuser2 0 0 :Regular User Two" \
#	"JOIN #protectedchannel2" \
#	"KICK #protectedchannel2 targetuser" \
#	"" \
#)
#
## Test 53: Invite user to channel
#op_invite_user=( \
#	"PASS password" \
#	"NICK inviter" \
#	"USER inviter 0 0 :Inviter" \
#	"JOIN #invitetest" \
#	"INVITE targetuser #invitetest" \
#	"" \
#)
#
## Test 54: Invite to non-existent channel
#op_invite_nonexistent_channel=( \
#	"PASS password" \
#	"NICK inviter2" \
#	"USER inviter2 0 0 :Inviter Two" \
#	"INVITE targetuser #nonexistentchannel" \
#	"" \
#)
#
## Test 55: Non-op invite attempt on invite-only channel
#op_non_op_invite=( \
#	"PASS password" \
#	"NICK regularinviter" \
#	"USER regularinviter 0 0 :Regular Inviter" \
#	"JOIN #privatechannel" \
#	"INVITE targetuser #privatechannel" \
#	"" \
#)
#
## Test 56: Set channel topic
#op_set_topic=( \
#	"PASS password" \
#	"NICK topicer" \
#	"USER topicer 0 0 :Topicer" \
#	"JOIN #topicset" \
#	"TOPIC #topicset :Welcome to our channel!" \
#	"" \
#)
#
## Test 57: Get channel topic
#op_get_topic=( \
#	"PASS password" \
#	"NICK topicreader" \
#	"USER topicreader 0 0 :Topic Reader" \
#	"JOIN #topicget" \
#	"TOPIC #topicget" \
#	"" \
#)
#
## Test 58: Non-op setting topic on restricted channel (should fail)
#op_non_op_topic_restricted=( \
#	"PASS password" \
#	"NICK restricteduser" \
#	"USER restricteduser 0 0 :Restricted User" \
#	"JOIN #restrictedtopic" \
#	"TOPIC #restrictedtopic :New topic" \
#	"" \
#)
#
## Test 59: Empty topic
#op_empty_topic=( \
#	"PASS password" \
#	"NICK emptytopicer" \
#	"USER emptytopicer 0 0 :Empty Topicer" \
#	"JOIN #emptytopic" \
#	"TOPIC #emptytopic :" \
#	"" \
#)
#
## Test 60: Very long topic
#op_long_topic=( \
#	"PASS password" \
#	"NICK longtopiccer" \
#	"USER longtopiccer 0 0 :Long Topicer" \
#	"JOIN #longtopic" \
#	"TOPIC #longtopic :This is a very long topic that tests how the server handles extended topic strings and buffer management for such data that could potentially exceed normal limits" \
#	"" \
#)

# EDGE CASES AND ERROR HANDLING

# Test 61: Commands before authentication
edge_commands_before_auth=( \
	"JOIN #test" \
	"PRIVMSG user :test" \
	"PASS password" \
	"NICK earlycommands" \
	"USER earlycommands 0 0 :Early Commands" \
	"" \
)

# Test 62: Repeated authentication attempts
edge_repeated_auth=( \
	"PASS password" \
	"NICK firstuser" \
	"USER firstuser 0 0 :First User" \
	"PASS password" \
	"NICK seconduser" \
	"USER seconduser 0 0 :Second User" \
	"" \
)

# Test 63: Duplicate nickname (same client)
edge_duplicate_nick_same=( \
	"PASS password" \
	"NICK dupnick" \
	"NICK dupnick" \
	"USER dupnick 0 0 :Dup Nick" \
	"" \
)

# Test 64: Unknown command
edge_unknown_command=( \
	"PASS password" \
	"NICK unknownuser" \
	"USER unknownuser 0 0 :Unknown User" \
	"UNKNOWNCMD param1 param2" \
	"" \
)

# Test 65: Malformed PRIVMSG (no colon)
edge_malformed_privmsg=( \
	"PASS password" \
	"NICK malformed" \
	"USER malformed 0 0 :Malformed" \
	"PRIVMSG #general" \
	"" \
)

# Test 66: Malformed JOIN (no channel)
edge_malformed_join=( \
	"PASS password" \
	"NICK badjoin" \
	"USER badjoin 0 0 :Bad Join" \
	"JOIN" \
	"" \
)

# Test 67: PING/PONG test
edge_ping_pong=( \
	"PASS password" \
	"NICK pinguser" \
	"USER pinguser 0 0 :Ping User" \
	"PING :testserver" \
	"" \
)

# Test 68: QUIT command
edge_quit=( \
	"PASS password" \
	"NICK quituser" \
	"USER quituser 0 0 :Quit User" \
	"QUIT :Goodbye!" \
	"" \
)

# Test 69: NAMES command
edge_names_command=( \
	"PASS password" \
	"NICK namesuser" \
	"USER namesuser 0 0 :Names User" \
	"JOIN #testchannel" \
	"NAMES #testchannel" \
	"" \
)

# Test 70: LIST command
edge_list_command=( \
	"PASS password" \
	"NICK listuser" \
	"USER listuser 0 0 :List User" \
	"LIST" \
	"" \
)

# Test 71: Partial line (no CRLF)
edge_partial_line=( \
	"PASS password" \
	"NICK partial" \
	"partial_incomplete_line_without_crlf" \
	"" \
)

# Test 72: Multiple commands in pipeline
edge_command_pipeline=( \
	"PASS password" \
	"NICK pipeuser" \
	"USER pipeuser 0 0 :Pipe User" \
	"JOIN #pipe1" \
	"JOIN #pipe2" \
	"JOIN #pipe3" \
	"PRIVMSG #pipe1 :Message to pipe1" \
	"PRIVMSG #pipe2 :Message to pipe2" \
	"PRIVMSG #pipe3 :Message to pipe3" \
	"" \
)

# Test 73: Case sensitivity of commands
edge_command_case=( \
	"pass password" \
	"nick lowercase" \
	"user lowercase 0 0 :Lowercase" \
	"join #testcase" \
	"" \
)

# Test 74: Tabs instead of spaces
edge_tabs_instead_spaces=( \
	"PASS	password" \
	"NICK	tabuser" \
	"USER	tabuser	0	0	:Tab User" \
	"JOIN	#tabtest" \
	"" \
)

# Test 75: Extra whitespace at end of line
edge_trailing_whitespace=( \
	"PASS password   " \
	"NICK trailing   " \
	"USER trailing 0 0 :Trailing   " \
	"JOIN #trailing   " \
	"" \
)


master_tests=( \
	"====== AUTHENTICATION TESTS ======" \
	"${auth_valid_sequence[@]}" \
	"${auth_with_realname_spaces[@]}" \
	"${auth_wrong_password[@]}" \
	"${auth_no_password[@]}" \
	"${auth_nick_before_pass[@]}" \
	"${auth_nick_only[@]}" \
	"${auth_user_only[@]}" \
	"${auth_duplicate_nick[@]}" \
	"${auth_nick_change[@]}" \
	"${auth_empty_nick[@]}" \
	"${auth_special_chars_nick[@]}" \
	"${auth_long_nick[@]}" \
	"${auth_empty_realname[@]}" \
	"${auth_long_realname[@]}" \
	"${auth_multiple_spaces_user[@]}" \
	"====== CHANNEL TESTS ======" \
	"${channel_simple_join[@]}" \
	"${channel_multiple_joins[@]}" \
	"${channel_join_no_hash[@]}" \
	"${channel_join_creates_new[@]}" \
	"${channel_join_with_key[@]}" \
	"${channel_simple_part[@]}" \
	"${channel_part_with_message[@]}" \
	"${channel_part_not_joined[@]}" \
	"${channel_privmsg_to_channel[@]}" \
	"${channel_privmsg_not_joined[@]}" \
	"${channel_empty_message[@]}" \
	"${channel_long_message[@]}" \
	"${channel_special_chars_message[@]}" \
	"${channel_escaped_message[@]}" \
	"====== PRIVATE MESSAGE TESTS ======" \
	"${privmsg_direct[@]}" \
	"${privmsg_nonexistent[@]}" \
	"${privmsg_empty[@]}" \
	"${privmsg_special[@]}" \
	"====== CHANNEL MODE TESTS ======" \
	"${mode_invite_only[@]}" \
	"${mode_remove_invite[@]}" \
	"${mode_set_key[@]}" \
	"${mode_remove_key[@]}" \
	"${mode_set_limit[@]}" \
	"${mode_remove_limit[@]}" \
	"${mode_topic_restricted[@]}" \
	"${mode_remove_topic_restriction[@]}" \
	"${mode_grant_op[@]}" \
	"${mode_remove_op[@]}" \
	"${mode_multiple_changes[@]}" \
	"${mode_combined[@]}" \
	"${mode_nonexistent_channel[@]}" \
	"${mode_non_op_attempt[@]}" \
	"${mode_invalid_character[@]}" \
	"====== OPERATOR TESTS ======" \
	"${op_kick_user[@]}" \
	"${op_kick_with_reason[@]}" \
	"${op_kick_nonexistent[@]}" \
	"${op_non_op_kick[@]}" \
	"${op_invite_user[@]}" \
	"${op_invite_nonexistent_channel[@]}" \
	"${op_non_op_invite[@]}" \
	"${op_set_topic[@]}" \
	"${op_get_topic[@]}" \
	"${op_non_op_topic_restricted[@]}" \
	"${op_empty_topic[@]}" \
	"${op_long_topic[@]}" \
	"====== EDGE CASES ======" \
	"${edge_commands_before_auth[@]}" \
	"${edge_repeated_auth[@]}" \
	"${edge_duplicate_nick_same[@]}" \
	"${edge_unknown_command[@]}" \
	"${edge_malformed_privmsg[@]}" \
	"${edge_malformed_join[@]}" \
	"${edge_ping_pong[@]}" \
	"${edge_quit[@]}" \
	"${edge_names_command[@]}" \
	"${edge_list_command[@]}" \
	"${edge_partial_line[@]}" \
	"${edge_command_pipeline[@]}" \
	"${edge_command_case[@]}" \
	"${edge_tabs_instead_spaces[@]}" \
	"${edge_trailing_whitespace[@]}" \
)

for command in "${master_tests[@]}"; do
	echo "$command"
done
