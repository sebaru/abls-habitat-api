# Abls-Habitat Project

Abls-Habitat is my own project to do home automation. it presents:

* one or more [Agents](https://github.com/sebaru/abls-habitat-agent), in house, to interact with sensors and outputs
* one [API](https://github.com/sebaru/abls-habitat-api) on SaaS, main process of project, to handle all of agents
* one [Console](https://github.com/sebaru/abls-habitat-console) to configure each element and develop [D.L.S module](https://docs.abls-habitat.fr/)
* one [Home](https://github/com/sebaru/abls-habitat-home) frontend for all users

This software is Work In Progress. It is a complete refund of all-in-one Watchdog Project.
I'm developing on my sparse-time, not so easy with little kid :-).

All detailed documentations [are here](https://docs.abls-habitat.fr)
Have a good day, Sebaru.

# What is Abls-Habitat API

This is the main API for working with [Abls-Habitat Console](https://github.com/sebaru/abls-habitat-console), [Abls-Habitat HOME](https://github.com/sebaru/abls-habitat-Home),
and [Abls-Habitat Agent](https://github.com/sebaru/abls-habitat-agent).

Main Goal is to separate things in my legacy Watchdog project and be able to simplify each components.

Maybe one of you may help if interested.

## Log facilities

`GET /log/facility/list` lists the domain's facility catalogue. It requires
domain access level 6 and returns `log_facilities`, sorted by name, with
`log_facility_id` (integer) and `log_facility` (string), plus
`nbr_log_facilities`. The catalogue covers the workspace's API, agents and
libraries; the legacy `SRC` project is excluded. Existing catalogue entries
and identifiers are preserved when adding missing facilities.

Schema version 124 clears all rows in `agent_log_facilities` and renames its
`log_facility` column to `log_facility_id`, an integer foreign key referencing
`log_facilities.log_facility_id`. The table name `log_facilities` is unchanged.
Existing per-facility debug selections must be configured again after the
migration; agent log levels are unchanged.

`POST /run/agent/config` continues to return facility names as
`log_facilities: [{"log_facility": "http"}]`, or an empty array when none are
selected. Running agents apply updated selections on their next configuration
load. No agent-side protocol change is required.
