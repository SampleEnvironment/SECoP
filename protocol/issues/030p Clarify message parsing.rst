SECoP Issue 30: Clarify Message parsing
=======================================

The problem
-----------
So far the specifiaction follows an *ignore-what-you-don't know* policy.
Implementing the protocol still requires to handle those cases.
After implementing several instance of clients and nodes, a pattern appears
in the parsing code.
It would ease implementations if several message could be treated the same.


Proposal
--------
treat following messages the same::

    "action"
    "action "                ; note: trailing space
    "action  "               ; note: two trailing spaces
    "action  null"           ; note: two space + json-null

::

    "action specifier"
    "action specifier "      ; note: trailing space
    "action specifier null"  ; note: data part == json-null

The message which MUST be sent by regular clients is the first line in the
above blocks. The lines below it SHOULD be treated the same.
This would allow a simplified parsing where the absence of a
message part can be treated the same as an empty part or a json-null.

The message format could then be written down in a human readable way (see :RFC:`5234`) as::

    serialized-message  = CR? action [ SPACE specifier [ SPACE data ]] CR? LF
    LF                  = %x0a                                 ; line feed character
    CR                  = %x0d                                 ; carriage return character
    SPACE               = %x20                                 ; SPACE character
    non-space           = %x21-%xff
    non-ctrl-char       = SPACE | non-space
    action              = any sequence of non-space * 1-N      ; MUST be valid utf-8, at least 1 character
    specifier           = any sequence of non-space * 0-N      ; MUST be valid utf-8, may be empty!
    data                = any sequence of non-ctrl-char * 0-N  ; MUST be valid json-text or be empty!

:note: The specification further restricts the possible content of action,
       specifier and data. The above is to illustrate the first step of
       deserialisation.

This could be writte as a regular expression like::

    ^\x0D?            ; matches 0 or 1 leading CR characters
    ([\x21-\xFF]+)    ; captures longest string of non-space (captures the action of the message, min 1 char)
    (?:\x20           ; open an non capturing group in the first SPACE character found
     ([\x21-\xFF]*)   ; captures longest string of non-space (captures the possibly empty specifier of the message)
     (?:\x20          ; open an non capturing group in the next SPACE character found
      ([\x20-\xFF]*)  ; capture remaining non-ctrl-char's
     )?               ; closes non captchuring group, match 1 or 0 occurences
    )?                ; closes non captchuring group, match 1 or 0 occurences
    \x0D?$            ; matches 0 or 1 trailing CR characters and the end of the line (i.e. the LF character)

:note: The regexp is written down line-by-line with added comments.

For actual use you should use it like::

       "^\x0D?([\x21-\xFF]+)(?:\x20([\x21-\xFF]*)(?:\x20([\x20-\xFF]*))?)?\x0D?$"
       For python you may also use named matches:
       "^\x0D?(?P<action>[\x21-\xFF]+)(?: (?P<specifier>[\x21-\xFF]*)(?: (?P<data>[\x20-\xFF]*))?)?\x0D?$"

Further deserialisation would then de-JSON-ify the data-part, if existing,
and map any '', null or not-matched groups to the same value for easier processing.
(i.e. finally a data-part of "null", "" or a missing one would mean all the same.)

Further processing (checking action against the list of predefined actions,
checking validity of the utf-8 or json data-part, checking data against the datatypes, etc....)
is not covered in this issue.


Discussion
----------
Essentially this is a more detailed write down of what was discussed previously
with the addition of treating a missing data part the same as a json-null
or an empty string.

This addition was not yet discussed, but would (if agreed upon) greatly simplify
message parsing of the current specification and provide a backwards compatible
fall-back, should a future revision want to distinguish between a missing part,
an empty part or a json-null.
