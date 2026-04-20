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
