# Check that the correct stride offset is extracted for the inner loop.
domain: "{ S[i, j] : exists u0, u1, u2 : 0 <= u0 <= 4 and 0 <= u1 <= 4 and 0 <= u2 <= 4 and i = 15u0 - 6u1 and j = -6u0 + 2u1 + 15u2 }"
