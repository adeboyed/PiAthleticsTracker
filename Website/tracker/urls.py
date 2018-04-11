from django.conf.urls import url 

from . import views

urlpatterns = [
    url(r'^get_status/', views.get_status, name='get_status'),
    url(r'^get_time/', views.get_time, name='get_time'),
    url(r'^start_race/', views.start_race, name='start_race'),
    url('', views.index, name='index'),
]
