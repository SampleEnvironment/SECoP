SECoP Issue 58: use JSON key, value for describe message (under discussion)
===========================================================================

Motivation
----------

We decided to use JSON for data transport, but we should use more features of it.
A module is an object with a name, a parameter and other things too.

There is one obvious way to implement a key, value hash in JSON(objects). We
access modules and parameters by name, so the natural way could be a
key, value access with a JSON object.

A human is no longer able to easily read the describe message. We should rely on
the help of a JSON library here. The added length of this message is around five
percent only, this could not be an issue.

Data mining algorithms will probably not assume that an array is a key, value pair.
We have a loss of information here.

Just because a JSON array can store different data types, does not mean it is a good idea.

The code becomes unnecessarily complicated, and it is hard to make nice solutions
with recursion etc.

In the SECoP rules we want to be explicit.

Proposal
--------

Use a JSON object for the module list, for accessibles inside modules and for datatypes.
Because of some JSON libraries do not keep the order of their key, value pairs, we need
an additional (TBD: **optional?**) "order_xyz" property to give that. There might be
libraries which keep the order, but they could handle this property either.

The module list inside the node is then a JSON object::

    {
        "equipment_id": "my equipment",
        "modules":
        {
            "mod1":
            {
                "datatype": ...
                "description": ...
                ...
            },
            "mod2": { ... }
        },
        "module_order": ["mod1", "mod2"]
    }

The parameter list inside the module is then also a JSON object::

    "module_name": {
        "accessibles":
        {
            "acc1": { ... },
            "acc2": { ... },
        },
        "accessible_order": ["acc1", "acc2"]
    }

The "datatype" property of an accessible has to be a JSON object too
(see `SECoP issue 55`_ in the original proposal)::

    {
        "type": "double",
        "unit": "K",
        ...
    }


.. _`SECoP issue 55`: 055%20Reformat%20Datatype%20description.rst


Discussion
----------

Some committee members do not see the need of change of syntax, as the interpretation
of the JSON description anyway needs some additional information.

As keeping the order of modules/accessibles is not essential for experiment,
one idea is to specify, that anyone who want to care about the order, must use
a JSON library which can keep the order. In principle, it is possible to write
JSON libraries in all languages which keep a determined order during encode and decode,
but as json.org defines that a JSON object is an unordered set of name/value pairs,
this might be confusing.

On the video meeting 2019-06-13 the following decision about ordering of
modules / accessibles was taken:

    ... change 'list of (key, value) pairs' back to JSON-object. Also, it should be
    noted in the specification, that the order of {} entries should be kept,
    i.e. SECoP uses a restricted version of JSON.

Later, in a discussion by E-mail, the idea of using a restricted version of JSON
was criticized.

Now again above option is proposed, with "module_order" / "accessible" order
possible changed to "order".

Proposals for the wording of the specification of optionality of the "order" property:

    For the functionality, the order of modules and accessibles does not matter.
    Order of such elements might be considered as a cosmetic issue.
    For debugging reasons, it is an advantage to use JSON libraries keeping the order
    on both the SEC-node and the ECS.
    A SEC-node programmer, who cares about the order, and does not want to rely on the
    behaviour of the JSON libraries, has to specify the "order" property.
    An ECS, who cares about the order, and does not want to rely on the
    behaviour of the JSON libraries, must interprete the "order" property and sort
    the elements accordingly. 


Decision
--------

Order of modules / accessibles:

tbd

Datatype property:

instead of above proposal, the formatting of datatypes is to change from
a 2 element list to a JSON-object with a single entry.
(essentially converting ["type",{...}] to {"type":{...}} )
