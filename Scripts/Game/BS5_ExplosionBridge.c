modded class SCR_WeaponBlastComponent
{
	override void OnWeaponFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)
	{
		super.OnWeaponFired(effectEntity, muzzle, projectileEntity);

		BS5_EchoDriverComponent driver = BS5_EchoRuntime.FindExplosionDriver(effectEntity, muzzle, projectileEntity);
		if (driver)
			driver.HandleExplosionFire(effectEntity, muzzle, projectileEntity);
	}
}

modded class SCR_ExplosiveTriggerComponent
{
	override void TriggeredInSafetyDistance(IEntity pHitEntity, inout vector outMat[3], IEntity damageSource, notnull Instigator instigator, string colliderName, float speed)
	{
		BS5_EchoRuntime.DispatchExplosionEffect(GetOwner(), pHitEntity, outMat, damageSource, instigator, "trigger");
		super.TriggeredInSafetyDistance(pHitEntity, outMat, damageSource, instigator, colliderName, speed);
	}
}

modded class SCR_PressureTriggerComponent
{
	override void TriggeredInSafetyDistance(IEntity pHitEntity, inout vector outMat[3], IEntity damageSource, notnull Instigator instigator, string colliderName, float speed)
	{
		BS5_EchoRuntime.DispatchExplosionEffect(GetOwner(), pHitEntity, outMat, damageSource, instigator, "pressure_trigger");
		super.TriggeredInSafetyDistance(pHitEntity, outMat, damageSource, instigator, colliderName, speed);
	}
}

class BS5_ExplosionEchoEffect : BaseProjectileEffect
{
	override void OnEffect(IEntity pHitEntity, inout vector outMat[3], IEntity damageSource, notnull Instigator instigator, string colliderName, float speed)
	{
		BS5_EchoRuntime.DispatchExplosionEffect(null, pHitEntity, outMat, damageSource, instigator, "projectile_effect");
		super.OnEffect(pHitEntity, outMat, damageSource, instigator, colliderName, speed);
	}
}
