/*
 * Table for debuging common I2C constants.
 *
 * Please note you might need a remove _INI hacks temporary to reveal
 * original value.
 */
DefinitionBlock ("", "SSDT", 2, "hack", "I2CD", 0x00000000)
{
    // Enable GPIO
    External (GPEN, FieldUnitObj)
    // Base address for GPIO config calculation
    External (SBRG, FieldUnitObj)
    // I2C Controller mode
    External (SMD0, FieldUnitObj)
    External (SMD1, FieldUnitObj)
    External (SMD2, FieldUnitObj)
    External (SMD3, FieldUnitObj)
    // I2C Interrupt mode
    External (SDM0, FieldUnitObj)
    External (SDM1, FieldUnitObj)
    // I2C Device type
    External (SDS0, FieldUnitObj)
    External (SDS1, FieldUnitObj)

    Scope (\_SB)
    {
        Device (I2CD)
        {
            Name (_HID, EisaId ("PNP0C02"))  // _HID: Hardware ID
            Name (_STA, 0x0F)  // _STA: Status

            If (CondRefOf (\GPEN))
            {
                Device (GPEN)
                {
                    Method (_ADR, 0, Serialized)  // _ADR: Address
                    {
                        Return (\GPEN)
                    }
                }
            }

            If (CondRefOf (\SBRG))
            {
                Device (SBRG)
                {
                    Method (_ADR, 0, Serialized)  // _ADR: Address
                    {
                        Return (\SBRG)
                    }
                }
            }

            If (CondRefOf (\SMD0))
            {
                Device (SMD0)
                {
                    Method (_ADR, 0, Serialized)  // _ADR: Address
                    {
                        Return (\SMD0)
                    }
                }
            }

            If (CondRefOf (\SMD1))
            {
                Device (SMD1)
                {
                    Method (_ADR, 0, Serialized)  // _ADR: Address
                    {
                        Return (\SMD1)
                    }
                }
            }

            If (CondRefOf (\SMD2))
            {
                Device (SMD2)
                {
                    Method (_ADR, 0, Serialized)  // _ADR: Address
                    {
                        Return (\SMD2)
                    }
                }
            }

            If (CondRefOf (\SMD3))
            {
                Device (SMD3)
                {
                    Method (_ADR, 0, Serialized)  // _ADR: Address
                    {
                        Return (\SMD3)
                    }
                }
            }

            If (CondRefOf (\SDM0))
            {
                Device (SDM0)
                {
                    Method (_ADR, 0, Serialized)  // _ADR: Address
                    {
                        Return (\SDM0)
                    }
                }
            }

            If (CondRefOf (\SDM1))
            {
                Device (SDM1)
                {
                    Method (_ADR, 0, Serialized)  // _ADR: Address
                    {
                        Return (\SDM1)
                    }
                }
            }

            If (CondRefOf (\SDS0))
            {
                Device (SDS0)
                {
                    Method (_ADR, 0, Serialized)  // _ADR: Address
                    {
                        Return (\SDS0)
                    }
                }
            }

            If (CondRefOf (\SDS1))
            {
                Device (SDS1)
                {
                    Method (_ADR, 0, Serialized)  // _ADR: Address
                    {
                        Return (\SDS1)
                    }
                }
            }
        }
    }
}

