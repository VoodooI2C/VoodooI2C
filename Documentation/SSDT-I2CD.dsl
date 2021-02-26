/*
 * Table for debuging common I2C constants.
 *
 * Please note you might need to remove _INI hacks temporary for
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
    External (GPDI, FieldUnitObj)
    External (GPLI, FieldUnitObj)
    External (SDM0, FieldUnitObj)
    External (SDM1, FieldUnitObj)
    // I2C Device type
    External (SDS0, FieldUnitObj)
    External (SDS1, FieldUnitObj)

    // Newer layout
    // I2C Controller mode
    External (IM00, FieldUnitObj)
    External (IM01, FieldUnitObj)
    External (IM02, FieldUnitObj)
    External (IM03, FieldUnitObj)
    External (IM04, FieldUnitObj)
    External (IM05, FieldUnitObj)
    // I2C Interrupt mode
    External (TPDM, FieldUnitObj)
    External (TPLM, FieldUnitObj)
    // I2C Device type
    External (TPDT, FieldUnitObj)
    External (TPLT, FieldUnitObj)
    // I2C Device base address
    External (TPDB, FieldUnitObj)
    External (TPLB, FieldUnitObj)
    // I2C Device HID
    External (TPDH, FieldUnitObj)
    External (TPLH, FieldUnitObj)
    // I2C Device Speed
    External (TPDS, FieldUnitObj)
    External (TPLS, FieldUnitObj)

    Scope (\_SB)
    {
        Device (I2CD)
        {
            Name (_HID, EisaId ("PNP0C02"))
            Name (_STA, 0x0F)  // _STA: Status

            If (CondRefOf (\GPEN))
            {
                Device (GPEN)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\GPEN)
                    }
                }
            }

            If (CondRefOf (\SBRG))
            {
                Device (SBRG)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\SBRG)
                    }
                }
            }

            If (CondRefOf (\SMD0))
            {
                Device (SMD0)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\SMD0)
                    }
                }
            }

            If (CondRefOf (\SMD1))
            {
                Device (SMD1)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\SMD1)
                    }
                }
            }

            If (CondRefOf (\SMD2))
            {
                Device (SMD2)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\SMD2)
                    }
                }
            }

            If (CondRefOf (\SMD3))
            {
                Device (SMD3)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\SMD3)
                    }
                }
            }

            If (CondRefOf (\GPDI))
            {
                Device (GPDI)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\GPDI)
                    }
                }
            }

            If (CondRefOf (\GPLI))
            {
                Device (GPLI)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\GPLI)
                    }
                }
            }

            If (CondRefOf (\SDM0))
            {
                Device (SDM0)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\SDM0)
                    }
                }
            }

            If (CondRefOf (\SDM1))
            {
                Device (SDM1)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\SDM1)
                    }
                }
            }

            If (CondRefOf (\SDS0))
            {
                Device (SDS0)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\SDS0)
                    }
                }
            }

            If (CondRefOf (\SDS1))
            {
                Device (SDS1)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\SDS1)
                    }
                }
            }

            If (CondRefOf (\IM00))
            {
                Device (IM00)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\IM00)
                    }
                }
            }

            If (CondRefOf (\IM01))
            {
                Device (IM01)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\IM01)
                    }
                }
            }

            If (CondRefOf (\IM02))
            {
                Device (IM02)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\IM02)
                    }
                }
            }

            If (CondRefOf (\IM03))
            {
                Device (IM03)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\IM03)
                    }
                }
            }

            If (CondRefOf (\IM04))
            {
                Device (IM04)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\IM04)
                    }
                }
            }

            If (CondRefOf (\IM05))
            {
                Device (IM05)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\IM05)
                    }
                }
            }

            If (CondRefOf (\TPDM))
            {
                Device (TPDM)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\TPDM)
                    }
                }
            }

            If (CondRefOf (\TPLM))
            {
                Device (TPLM)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\TPLM)
                    }
                }
            }

            If (CondRefOf (\TPDT))
            {
                Device (TPDT)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\TPDT)
                    }
                }
            }

            If (CondRefOf (\TPLT))
            {
                Device (TPLT)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\TPLT)
                    }
                }
            }

            If (CondRefOf (\TPDB))
            {
                Device (TPDB)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\TPDB)
                    }
                }
            }

            If (CondRefOf (\TPLB))
            {
                Device (TPLB)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\TPLB)
                    }
                }
            }

            If (CondRefOf (\TPDH))
            {
                Device (TPDH)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\TPDH)
                    }
                }
            }

            If (CondRefOf (\TPLH))
            {
                Device (TPLH)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\TPLH)
                    }
                }
            }

            If (CondRefOf (\TPDS))
            {
                Device (TPDS)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\TPDS)
                    }
                }
            }

            If (CondRefOf (\TPLS))
            {
                Device (TPLS)
                {
                    Method (_ADR, 0, Serialized)
                    {
                        Return (\TPLS)
                    }
                }
            }
        }
    }
}

