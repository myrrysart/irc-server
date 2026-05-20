#!/bin/bash

# IRC Server Stress and Fuzzing Tests

# Test 1: Very long nickname (stress nick buffer)
stress_long_nick=( \
	"PASS password" \
	"NICK verylongnicknametotestbufferlimitsverylongnicknametotestbufferlimitsverylongnicknametotestbufferlimits" \
	"USER verylongnicknametotestbufferlimits 0 0 :Long Nick" \
	"" \
)

# Test 2: Very long username
stress_long_username=( \
	"PASS password" \
	"NICK longusertest" \
	"USER verylongusernametotestbufferlimitsverylongusernametotestbufferlimitsverylongusernametotestbufferlimits 0 0 :Long User" \
	"" \
)

# Test 3: Very long real name (512+ bytes)
stress_long_realname=( \
	"PASS password" \
	"NICK realnametest" \
	"USER realnametest 0 0 :This is an extremely long real name that is designed to test how the server handles very long strings in the real name field. It should contain a lot of text to really push the boundaries of what the IRC protocol might support. We are testing buffer management and ensuring the server doesnt crash or leak memory when handling such extreme inputs. The goal is to find where the limits are and how the server responds to them." \
	"" \
)

# Test 4: Very long channel name
stress_long_channel=( \
	"PASS password" \
	"NICK longchannel" \
	"USER longchannel 0 0 :Long Channel" \
	"JOIN #verylongchannelnametotestbufferlimitsverylongchannelnametotestbufferlimitsverylongchannelnametotestbufferlimits" \
	"" \
)

# Test 5: Very long message (1000+ chars)
stress_long_message=( \
	"PASS password" \
	"NICK longmsg" \
	"USER longmsg 0 0 :Long Message" \
	"JOIN #stress" \
	"PRIVMSG #stress :Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit." \
	"" \
)

# Test 6: Message with null bytes (potential exploit)
stress_null_bytes_in_message=( \
	"PASS password" \
	"NICK nulltest" \
	"USER nulltest 0 0 :Null Test" \
	"JOIN #stress" \
	"PRIVMSG #stress :Message with null\\x00byte\\x00embedded" \
	"" \
)

# Test 7: Very long password
stress_long_password=( \
	"PASS verylongpasswordtotestbufferlimitsverylongpasswordtotestbufferlimitsverylongpasswordtotestbufferlimits" \
	"NICK longpasstest" \
	"USER longpasstest 0 0 :Long Pass" \
	"" \
)

# Test 8: Very long topic
stress_long_topic=( \
	"PASS password" \
	"NICK topicstress" \
	"USER topicstress 0 0 :Topic Stress" \
	"JOIN #stressopic" \
	"TOPIC #stressopic :This is an extremely long topic that should test how the server handles very large topic strings. Topics can contain a lot of information and some servers might have different limits. We need to test what happens when we try to set a topic that exceeds normal expectations or buffer sizes. The topic can contain special characters and formatting that might also affect buffer handling." \
	"" \
)

# Test 9: Very long channel key
stress_long_key=( \
	"PASS password" \
	"NICK keystress" \
	"USER keystress 0 0 :Key Stress" \
	"JOIN #keytest" \
	"MODE #keytest +k verylongkeytotestbufferlimitsverylongkeytotestbufferlimitsverylongkeytotestbufferlimits" \
	"" \
)

# Test 10: Rapid fire commands (no delays)
stress_rapid_commands=( \
	"PASS password" \
	"NICK rapid1" \
	"NICK rapid2" \
	"NICK rapid3" \
	"USER rapid3 0 0 :Rapid" \
	"JOIN #rapid1" \
	"JOIN #rapid2" \
	"JOIN #rapid3" \
	"PRIVMSG #rapid1 :msg1" \
	"PRIVMSG #rapid2 :msg2" \
	"PRIVMSG #rapid3 :msg3" \
	"PART #rapid1" \
	"PART #rapid2" \
	"PART #rapid3" \
	"" \
)

# BINARY/SPECIAL CHARACTER TESTS

# Test 11: High-bit ASCII characters
stress_high_ascii=( \
	"PASS password" \
	"NICK ascii" \
	"USER ascii 0 0 :High ASCII ÄÖÜßñ" \
	"JOIN #ascii" \
	"PRIVMSG #ascii :Special chars: ½ © ® ™ € £ ¥" \
	"" \
)

# Test 12: Control characters in message
stress_control_chars=( \
	"PASS password" \
	"NICK control" \
	"USER control 0 0 :Control" \
	"JOIN #control" \
	"PRIVMSG #control :Control\\x01chars\\x02test\\x03" \
	"" \
)

# Test 13: Backslash escaping
stress_backslash=( \
	"PASS password" \
	"NICK backslash" \
	"USER backslash 0 0 :Backslash Test" \
	"JOIN #backslash" \
	"PRIVMSG #backslash :Backslash test: \\ \\\\ \\r \\n \\t" \
	"" \
)

# PROTOCOL EDGE CASES

# Test 14: Missing CRLF line ending
stress_no_crlf=( \
	"PASS password\nNICK nocrlf\nUSER nocrlf 0 0 :NoCRLF" \
	"" \
)

# Test 15: Multiple CRLF sequences
stress_multiple_crlf=( \
	"PASS password\r\n\r\n\r\n" \
	"NICK multicrlf" \
	"USER multicrlf 0 0 :Multi CRLF" \
	"" \
)

# Test 16: Only CRLF (empty lines)
stress_only_crlf=( \
	"PASS password" \
	"" \
	"" \
	"" \
	"NICK emptylines" \
	"USER emptylines 0 0 :Empty Lines" \
	"" \
)

# Test 17: Colon without trailing param
stress_colon_no_param=( \
	"PASS password" \
	"NICK colontest" \
	"USER colontest 0 0 :Colon Test" \
	"JOIN #colontest" \
	"PRIVMSG #colontest :" \
	"" \
)

# Test 18: Multiple colons in message
stress_multiple_colons=( \
	"PASS password" \
	"NICK colonmulti" \
	"USER colonmulti 0 0 :Colon Multi" \
	"JOIN #colonmulti" \
	"PRIVMSG #colonmulti :::::::multiple colons:::::" \
	"" \
)

# Test 19: Parameter without space separation
stress_no_space_params=( \
	"PASS password" \
	"NICK nospace" \
	"USER nospace 0 0 :No Space" \
	"JOINtest" \
	"" \
)

# Test 20: Too many parameters
stress_too_many_params=( \
	"PASS password" \
	"NICK manyparams" \
	"USER manyparams extra extra extra 0 0 :Many Params" \
	"JOIN #test extra1 extra2 extra3 extra4" \
	"" \
)

# STATE MUTATION TESTS

# Test 21: Rapid nick changes
stress_nick_spam=( \
	"PASS password" \
	"NICK nick1" \
	"NICK nick2" \
	"NICK nick3" \
	"NICK nick4" \
	"NICK nick5" \
	"NICK nick6" \
	"NICK nick7" \
	"NICK nick8" \
	"NICK nick9" \
	"NICK nick10" \
	"USER nick10 0 0 :Nick Spam" \
	"" \
)

# Test 22: Join/Part spam
stress_join_part_spam=( \
	"PASS password" \
	"NICK joinpart" \
	"USER joinpart 0 0 :Join Part" \
	"JOIN #test1" \
	"PART #test1" \
	"JOIN #test2" \
	"PART #test2" \
	"JOIN #test3" \
	"PART #test3" \
	"JOIN #test4" \
	"PART #test4" \
	"JOIN #test5" \
	"PART #test5" \
	"" \
)

# Test 23: Mode spam
stress_mode_spam=( \
	"PASS password" \
	"NICK modesp" \
	"USER modespam 0 0 :Mode Spam" \
	"JOIN #modetest" \
	"MODE #modetest +i" \
	"MODE #modetest -i" \
	"MODE #modetest +i" \
	"MODE #modetest -i" \
	"MODE #modetest +t" \
	"MODE #modetest -t" \
	"MODE #modetest +t" \
	"" \
)

# Test 24: Channel switching spam
stress_channel_spam=( \
	"PASS password" \
	"NICK chspam" \
	"USER channelspam 0 0 :Channel Spam" \
	"JOIN #chan1" \
	"JOIN #chan2" \
	"JOIN #chan3" \
	"JOIN #chan4" \
	"JOIN #chan5" \
	"JOIN #chan6" \
	"JOIN #chan7" \
	"JOIN #chan8" \
	"JOIN #chan9" \
	"JOIN #chan10" \
	"" \
)

# AUTHENTICATION BYPASS ATTEMPTS

# Test 25: PASS with colon in password
auth_colon_password=( \
	"PASS pass:word:with:colons" \
	"NICK colonpass" \
	"USER colonpass 0 0 :Colon Pass" \
	"" \
)

# Test 26: PASS with spaces (testing parameter parsing)
auth_space_password=( \
	"PASS pass with spaces" \
	"NICK spacepass" \
	"USER spacepass 0 0 :Space Pass" \
	"" \
)

# Test 27: Commands with leading/trailing spaces
stress_spaces_commands=( \
	"  PASS password  " \
	"  NICK spacetest  " \
	"  USER spacetest 0 0 :Space Test  " \
	"  JOIN #spacetest  " \
	"  PRIVMSG #spacetest :message  " \
	"" \
)

# Test 28: Mixed case commands
stress_mixed_case=( \
	"PaSs password" \
	"NiCk mixedcase" \
	"UsEr mixedcase 0 0 :Mixed Case" \
	"jOiN #mixedcase" \
	"pRiVmSg #mixedcase :msg" \
	"" \
)

# Test 29: Command-like strings in messages (should NOT be executed)
stress_command_in_message=( \
	"PASS password" \
	"NICK cmdmsg" \
	"USER cmdmsg 0 0 :Cmd Message" \
	"JOIN #cmdtest" \
	"PRIVMSG #cmdtest :PASS command in message" \
	"PRIVMSG #cmdtest :NICK shouldntchange" \
	"PRIVMSG #cmdtest :JOIN #shouldntjoin" \
	"" \
)

# Test 30: Incomplete commands
stress_incomplete_commands=( \
	"PASS password" \
	"NICK incomplete" \
	"USER incomplete 0 0 :Incomplete" \
	"JOIN" \
	"PRIVMSG" \
	"MODE" \
	"KICK" \
	"INVITE" \
	"TOPIC" \
	"" \
)

# MEMORY/RESOURCE STRESS

# Test 31: Deep command nesting simulation
stress_nesting=( \
	"PASS password" \
	"NICK nesting" \
	"USER nesting 0 0 :Nesting Test" \
	"JOIN #nest1" \
	"JOIN #nest2" \
	"JOIN #nest3" \
	"JOIN #nest4" \
	"JOIN #nest5" \
	"JOIN #nest6" \
	"JOIN #nest7" \
	"JOIN #nest8" \
	"PRIVMSG #nest1 :msg1" \
	"PRIVMSG #nest2 :msg2" \
	"PRIVMSG #nest3 :msg3" \
	"PRIVMSG #nest4 :msg4" \
	"PRIVMSG #nest5 :msg5" \
	"PRIVMSG #nest6 :msg6" \
	"PRIVMSG #nest7 :msg7" \
	"PRIVMSG #nest8 :msg8" \
	"" \
)

# Test 32: Repeated allocation/deallocation pattern
stress_alloc_pattern=( \
	"PASS password" \
	"NICK alloctest" \
	"USER alloctest 0 0 :Alloc Test" \
	"JOIN #temp1" \
	"PART #temp1" \
	"JOIN #temp2" \
	"PART #temp2" \
	"JOIN #temp3" \
	"PART #temp3" \
	"JOIN #temp4" \
	"PART #temp4" \
	"JOIN #temp5" \
	"PART #temp5" \
	"JOIN #perm" \
	"" \
)

# Test 33: Many mode parameters
stress_many_mode_params=( \
	"PASS password" \
	"NICK manymodes" \
	"USER manymodes 0 0 :Many Modes" \
	"JOIN #manytest" \
	"MODE #manytest +oooo user1 user2 user3 user4" \
	"MODE #manytest -oooo user1 user2 user3 user4" \
	"" \
)

# DATA INTEGRITY TESTS

# Test 34: Send message immediately after join
stress_immed_message=( \
	"PASS password" \
	"NICK immed" \
	"USER immed 0 0 :Immediate" \
	"JOIN #immedtest" \
	"PRIVMSG #immedtest :Sent right after join" \
	"" \
)

# Test 35: Quit in middle of operations
stress_quit_abrupt=( \
	"PASS password" \
	"NICK quitabrupt" \
	"USER quitabrupt 0 0 :Quit Abrupt" \
	"JOIN #quitfirst" \
	"JOIN #quitsecond" \
	"PRIVMSG #quitfirst :Message before quit" \
	"QUIT :Sudden departure" \
	"" \
)

# Test 36: Reconnect with same nick
stress_reconnect_same_nick=( \
	"PASS password" \
	"NICK reconnect" \
	"USER reconnect 0 0 :Reconnect" \
	"QUIT :First session" \
	"PASS password" \
	"NICK reconnect" \
	"USER reconnect 0 0 :Reconnect Again" \
	"" \
)

# Test 37: Join same channel multiple times
stress_join_duplicate=( \
	"PASS password" \
	"NICK dupjoin" \
	"USER dupjoin 0 0 :Dup Join" \
	"JOIN #duptest" \
	"JOIN #duptest" \
	"JOIN #duptest" \
	"PRIVMSG #duptest :Message after duplicate joins" \
	"" \
)

# Test 38: Mode same channel multiple times rapid
stress_mode_same=( \
	"PASS password" \
	"NICK modesame" \
	"USER modesame 0 0 :Mode Same" \
	"JOIN #modesametest" \
	"MODE #modesametest +i" \
	"MODE #modesametest +i" \
	"MODE #modesametest +i" \
	"MODE #modesametest -i" \
	"MODE #modesametest -i" \
	"" \
)

stress_fuzzing_tests=( \
	"====== BUFFER STRESS TESTS ======" \
	"${stress_long_nick[@]}" \
	"${stress_long_username[@]}" \
	"${stress_long_realname[@]}" \
	"${stress_long_channel[@]}" \
	"${stress_long_message[@]}" \
	"${stress_null_bytes_in_message[@]}" \
	"${stress_long_password[@]}" \
	"${stress_long_topic[@]}" \
	"${stress_long_key[@]}" \
	"${stress_rapid_commands[@]}" \
	"====== BINARY/SPECIAL CHARACTER TESTS ======" \
	"${stress_high_ascii[@]}" \
	"${stress_control_chars[@]}" \
	"${stress_backslash[@]}" \
	"====== PROTOCOL EDGE CASES ======" \
	"${stress_no_crlf[@]}" \
	"${stress_multiple_crlf[@]}" \
	"${stress_only_crlf[@]}" \
	"${stress_colon_no_param[@]}" \
	"${stress_multiple_colons[@]}" \
	"${stress_no_space_params[@]}" \
	"${stress_too_many_params[@]}" \
	"====== STATE MUTATION TESTS ======" \
	"${stress_nick_spam[@]}" \
	"${stress_join_part_spam[@]}" \
	"${stress_mode_spam[@]}" \
	"${stress_channel_spam[@]}" \
	"====== AUTHENTICATION BYPASS ATTEMPTS ======" \
	"${auth_colon_password[@]}" \
	"${auth_space_password[@]}" \
	"${stress_spaces_commands[@]}" \
	"${stress_mixed_case[@]}" \
	"${stress_command_in_message[@]}" \
	"${stress_incomplete_commands[@]}" \
	"====== MEMORY/RESOURCE STRESS ======" \
	"${stress_nesting[@]}" \
	"${stress_alloc_pattern[@]}" \
	"${stress_many_mode_params[@]}" \
	"====== DATA INTEGRITY TESTS ======" \
	"${stress_immed_message[@]}" \
	"${stress_quit_abrupt[@]}" \
	"${stress_reconnect_same_nick[@]}" \
	"${stress_join_duplicate[@]}" \
	"${stress_mode_same[@]}" \
)

for command in "${stress_fuzzing_tests[@]}"; do
	echo "$command"
done
