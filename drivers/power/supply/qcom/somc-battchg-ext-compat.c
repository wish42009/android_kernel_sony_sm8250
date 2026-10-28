// SPDX-License-Identifier: GPL-2.0-only
/*
 * XperiaGSI: /sys/class/battchg_ext/smart_charging_{activation,interruption} for Sony's off-mode charger UI.
 *
 * Sony's chargemon (the off-mode charging screen, /vendor/bin/chargemon) implements off-mode battery care with
 * the battchg_ext class of newer Xperia kernels: with MiscTA unit 2318 set (enabled, target level), it writes
 * "1" to smart_charging_activation, reads it back, then writes smart_charging_interruption = 1 once the battery
 * reaches the target level and 0 below it. This kernel has the same controls on the battery_ext power supply
 * (qpnp-smb5, CONFIG_SOMC_CHARGER_EXTENSION), which init.sony-platform.rc already uses while Android runs.
 * These two class attributes pass reads and writes through to battery_ext, nothing else; chargemon's other
 * battchg_ext node (bootup_shutdown_phase) has no counterpart here and is left out, as before.
 *
 * Mode 0664 root:root: chargemon runs as user system with the root group. battery_ext is looked up on every
 * access, so the order of driver probing does not matter; without it the attributes return -ENODEV.
 */

#define pr_fmt(fmt) "battchg_ext_compat: " fmt

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/power_supply.h>

static int battchg_ext_get(enum power_supply_property psp, int *out)
{
	struct power_supply *psy = power_supply_get_by_name("battery_ext");
	union power_supply_propval val = { 0, };
	int rc;

	if (!psy)
		return -ENODEV;
	rc = power_supply_get_property(psy, psp, &val);
	power_supply_put(psy);
	if (!rc)
		*out = val.intval;
	return rc;
}

static int battchg_ext_set(enum power_supply_property psp, int in)
{
	struct power_supply *psy = power_supply_get_by_name("battery_ext");
	union power_supply_propval val = { .intval = in, };
	int rc;

	if (!psy)
		return -ENODEV;
	rc = power_supply_set_property(psy, psp, &val);
	power_supply_put(psy);
	return rc;
}

static ssize_t battchg_ext_show(enum power_supply_property psp, char *buf)
{
	int v = 0;
	int rc = battchg_ext_get(psp, &v);

	return rc ? rc : scnprintf(buf, PAGE_SIZE, "%d\n", v);
}

static ssize_t battchg_ext_store(enum power_supply_property psp, const char *buf, size_t count)
{
	int v, rc;

	rc = kstrtoint(buf, 10, &v);
	if (rc)
		return rc;
	rc = battchg_ext_set(psp, v);
	return rc ? rc : count;
}

static ssize_t smart_charging_activation_show(struct class *c, struct class_attribute *attr, char *buf)
{
	return battchg_ext_show(POWER_SUPPLY_PROP_SMART_CHARGING_ACTIVATION, buf);
}

static ssize_t smart_charging_activation_store(struct class *c, struct class_attribute *attr,
		const char *buf, size_t count)
{
	return battchg_ext_store(POWER_SUPPLY_PROP_SMART_CHARGING_ACTIVATION, buf, count);
}

static ssize_t smart_charging_interruption_show(struct class *c, struct class_attribute *attr, char *buf)
{
	return battchg_ext_show(POWER_SUPPLY_PROP_SMART_CHARGING_INTERRUPTION, buf);
}

static ssize_t smart_charging_interruption_store(struct class *c, struct class_attribute *attr,
		const char *buf, size_t count)
{
	return battchg_ext_store(POWER_SUPPLY_PROP_SMART_CHARGING_INTERRUPTION, buf, count);
}

static struct class_attribute class_attr_smart_charging_activation =
	__ATTR(smart_charging_activation, 0664, smart_charging_activation_show, smart_charging_activation_store);
static struct class_attribute class_attr_smart_charging_interruption =
	__ATTR(smart_charging_interruption, 0664, smart_charging_interruption_show,
	       smart_charging_interruption_store);

static struct attribute *battchg_ext_class_attrs[] = {
	&class_attr_smart_charging_activation.attr,
	&class_attr_smart_charging_interruption.attr,
	NULL,
};
ATTRIBUTE_GROUPS(battchg_ext_class);

static struct class battchg_ext_class = {
	.name = "battchg_ext",
	.owner = THIS_MODULE,
	.class_groups = battchg_ext_class_groups,
};

static int __init battchg_ext_compat_init(void)
{
	int rc = class_register(&battchg_ext_class);

	if (rc)
		pr_err("class_register failed: %d\n", rc);	/* only chargemon's off-mode battery care is affected */
	return 0;
}
late_initcall(battchg_ext_compat_init);
