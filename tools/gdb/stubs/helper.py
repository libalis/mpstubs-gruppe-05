
def compact_entries(entries, cmp_eq):
    l = []
    for idx, entry in entries:
        # prepare first item in list
        if len(l) == 0:
            l.append((idx, idx, entry))
            continue

        # check for duplicate entry
        if cmp_eq(l[-1][2], entry):
            old = l.pop()
            l.append((old[0], idx, old[2]))
            continue

        l.append((idx, idx, entry))
    return l

def bits(value, start, nbits):
    # drop any bits positioned higher than (start + nbits)
    mask = 1 << (start + nbits)  # mind the overflow in any other language without bigints!
    mask = mask - 1
    value = value & mask

    # select only nbits
    mask = (1 << nbits) - 1
    mask = mask << start
    value = value & mask

    # shift result accordingly
    value = value >> start

    return value
